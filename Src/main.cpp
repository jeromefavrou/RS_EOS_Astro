#define __DEBUG_MODE

#define MNT_CONF_PATH ".mnt_configure"
#define CLIENT_CONF_PATH ".server_configure"

#define Id_Socket 0

#include "Remote_controle_apn.hpp"
#include "serveur.cpp"

void check_acknowledge(VCHAR const & rep_tram)
{
        if(rep_tram[0]==Tram::Com_bytes::SOH && rep_tram.back()==Tram::Com_bytes::EOT)
        {
            if(static_cast<int>(rep_tram[1])==Tram::Com_bytes::NAK)
            {
                std::string er("");
                for(auto &t : rep_tram)
                {
                    if(t==Tram::Com_bytes::EOT)
                        break;
                    if(t==Tram::Com_bytes::SOH)
                        continue;

                    er+=t;
                }

                throw Error(5,"erreur du serveur: "+er,Error::niveau::WARNING);
            }
            else
                return ;
        }
        else
            throw Error(7,"header et footer tram non respecté par le serveur: ",Error::niveau::ERROR);

    throw Error(9,"Erreur inconnue du avec le serveur: ",Error::niveau::ERROR);
}

void init_configure(struct gp2::mnt & _mount)
{
    std::fstream If(MNT_CONF_PATH,std::ios::in);

    if(!If || If.bad() || If.fail())
        throw Error(1,"erreur a la lecture de \""+std::string(MNT_CONF_PATH)+"\"",Error::niveau::WARNING);

    std::getline(If,_mount.cmd);
    std::getline(If,_mount.path);
}

int main(int argc,char ** argv)
{
    struct gp2::mnt _mount;

    bool as_client=false;

    RC_Apn apn;
    CSocketTCPServeur Server;

    #ifdef __DEBUG_MODE
    apn.debug_mode=true;
    #else
    apn.debug_mode=false;
    #endif // __DEBUG_MODE

    apn.tcp_client=false;
    apn.no_download= false;
    apn.no_delete= false;
    apn.older= false;

    try
    {
        if(system(nullptr));
        else
            throw Error(1,"les commandes system ne peuvent etre utilise",Error::niveau::FATAL_ERROR);

        init_configure(_mount);
    }
    catch(Error & e)
    {
        std::cerr << e.what() << std::endl;

        if(e.get_niveau()!=Error::niveau::WARNING)
            return -1;

        #ifndef WIN32
        _mount.cmd="gio mount";
        _mount.path="/run/user/1000/gvfs";
        #elif
        return -1;
        #endif // WIN32
    }

    try
    {
        Server.NewSocket(Id_Socket);
        Server.BindServeur(Id_Socket,INADDR_ANY,6789);
        Server.Listen(Id_Socket,1);

        #ifdef __DEBUG_MODE
            std::clog << "en attente de client" << std::endl;
        #endif // __DEBUG_MODE
        Server.AcceptClient(Id_Socket,0);
        #ifdef __DEBUG_MODE
            std::clog << "client connecté" << std::endl;
        #endif // __DEBUG_MODE
        as_client=true;
    }
    catch(Error & e)
    {
        std::cerr << e.what() << std::endl;

        if(e.get_niveau()!=Error::niveau::WARNING)
            return -1;
    }

    while(true)
    {
        Tram Request;
        Tram Respond;
        try
        {
            if(Server.Read<2048>(0,Request.get_data())<=0)
                throw Error(1,"client deconnecté",Error::niveau::WARNING);
        }
        catch(Error & e)
        {
            std::cerr << e.what() << std::endl;

            if(e.get_niveau()!=Error::niveau::WARNING)
                return -1;

            #ifdef __DEBUG_MODE
                std::clog << "attente de reconnexion du client" << std::endl;
            #endif // __DEBUG_MODE

            Server.AcceptClient(Id_Socket,0);

            #ifdef __DEBUG_MODE
                std::clog << "client reconnecté" << std::endl;
            #endif // __DEBUG_MODE

            continue;
        }

        try
        {
            #ifdef __DEBUG_MODE
                std::clog << "Tram: "<< std::hex;
                for(auto & i:Request.get_data())
                    std::clog <<"0x"<< static_cast<int>(i) << " ";
                std::clog  << std::dec << std::endl;
            #endif // __DEBUG_MODE

            check_acknowledge(Request.get_c_data());

            Respond+=(char)Tram::Com_bytes::SOH;
            Respond+=(char)Tram::Com_bytes::ACK;
            Respond+=(char)Tram::Com_bytes::EOT;

            Server.Write(Id_Socket,Respond.get_c_data());
        }
        catch(Error & e)
        {
            std::cerr << e.what() << std::endl;

            if(e.get_niveau()!=Error::niveau::WARNING)
                return -1;
        }

        std::this_thread::sleep_for(std::chrono::duration<int,std::milli>(20));
    }

    return 0;
}
