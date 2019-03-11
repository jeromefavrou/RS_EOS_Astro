#define __DEBUG_MODE

#define MNT_CONF_PATH ".mnt_configure"
#define CLIENT_CONF_PATH ".server_configure"

#define Id_Socket 0

#include "Remote_controle_apn.hpp"
#include "serveur.cpp"

Tram Read_Tram(char const ending_byte,CSocketTCPServeur & Server,int id_server,int const _time_out)
{
    Tram data;

    bool Do=true;


    while(Do)
    {
        VCHAR tps;

        Server.Read<2048>(id_server,tps);

        data+=tps;

        if(tps.back()==ending_byte)
            break;
    }

    return data;
}

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
void _get_conf(gp2::Conf_param const& cp,VCHAR const & r_data,Tram & t_data,gp2::Data & gc)
{
    try
    {
        gp2::Get_config(cp,gc);

        t_data+=(char)Tram::Com_bytes::ACK;

        for(auto & i :gc)
        {
            t_data+=i;
            t_data+=(char)Tram::Com_bytes::US;
        }
    }
    catch(Error & e)
    {
        std::cerr<< e.what() <<std::endl;

        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void _set_conf(gp2::Conf_param const& cp,VCHAR const & r_data,Tram & t_data)
{
    try
    {
        std::string value("");

        for(auto i=3;i<r_data.size();i++)
        {
            if(r_data[i]==Tram::Com_bytes::EOT)
            {
                gp2::Set_config(cp,value,false);
                break;
            }

            value+=(char)r_data[i];
        }

        t_data+=(char)Tram::Com_bytes::ACK;
    }
    catch(Error & e)
    {
        std::cerr<< e.what() <<std::endl;

        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void _remove(VCHAR const & r_data,Tram & t_data)
{
    try
    {
        std::string value("");

        for(auto i=2;i<r_data.size();i++)
        {
            if(r_data[i]==Tram::Com_bytes::EOT)
                break;

            value+=(char)r_data[i];
        }

        gp2::Delete_file(value,false);

        t_data+=(char)Tram::Com_bytes::ACK;
    }
    catch(Error & e)
    {
        std::cerr<< e.what() <<std::endl;

        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void process(VCHAR const & r_data,Tram & t_data)
{
    struct gp2::mnt _mnt{"gio mount",""};
    t_data.clear();
    t_data+=(char)Tram::Com_bytes::SOH;

    if(r_data[1]==(char)RC_Apn::Com_bytes::Check_Apn)
    {
        gp2::Data device;
        gp2::Auto_detect(device);

        if(device.size()>0)
        {
            t_data+=(char)Tram::Com_bytes::ACK;
            gp2::Unmount(_mnt);
        }
        else
        {
            t_data+=(char)Tram::Com_bytes::NAK;
            t_data+="aucun apn detecté";
        }
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Older)
    {
        t_data+=(char)Tram::Com_bytes::ACK;
    }

    else if(r_data[1]==(char)RC_Apn::Com_bytes::Debug_mode)
    {
        t_data+=(char)Tram::Com_bytes::ACK;
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Get_Config)//simplifiable par une fonction parsé
    {
        gp2::Data gc;

        if(r_data[2]==RC_Apn::Com_bytes::Aperture)
            _get_conf(gp2::Conf_param::APERTURE,r_data,t_data,gc);
        else if(r_data[2]==RC_Apn::Com_bytes::Shutterspeed)
            _get_conf(gp2::Conf_param::SHUTTERSPEED,r_data,t_data,gc);
        else if(r_data[2]==RC_Apn::Com_bytes::Iso)
            _get_conf(gp2::Conf_param::ISO,r_data,t_data,gc);
        else if(r_data[2]==RC_Apn::Com_bytes::Format)
            _get_conf(gp2::Conf_param::FORMAT,r_data,t_data,gc);
        else if(r_data[2]==RC_Apn::Com_bytes::Target)
            _get_conf(gp2::Conf_param::TARGET,r_data,t_data,gc);
        else if(r_data[2]==RC_Apn::Com_bytes::White_balance)
            _get_conf(gp2::Conf_param::WHITE_BALANCE,r_data,t_data,gc);
        else if(r_data[2]==RC_Apn::Com_bytes::Picture_style)
            _get_conf(gp2::Conf_param::PICTURE_STYLE,r_data,t_data,gc);
        else
        {
            t_data+=(char)Tram::Com_bytes::NAK;
            t_data+="demande de Get_Config inconnue: ";
            t_data+=r_data[2];
        }
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Set_Config)
    {
        if(r_data[2]==(char)RC_Apn::Com_bytes::Aperture)
            _set_conf(gp2::Conf_param::APERTURE,r_data,t_data);
        else if(r_data[2]==RC_Apn::Com_bytes::Shutterspeed)
            _set_conf(gp2::Conf_param::SHUTTERSPEED,r_data,t_data);
        else if(r_data[2]==RC_Apn::Com_bytes::Iso)
            _set_conf(gp2::Conf_param::ISO,r_data,t_data);
        else if(r_data[2]==RC_Apn::Com_bytes::Format)
            _set_conf(gp2::Conf_param::FORMAT,r_data,t_data);
        else if(r_data[2]==RC_Apn::Com_bytes::Target)
            _set_conf(gp2::Conf_param::TARGET,r_data,t_data);
        else if(r_data[2]==RC_Apn::Com_bytes::White_balance)
            _set_conf(gp2::Conf_param::WHITE_BALANCE,r_data,t_data);
        else if(r_data[2]==RC_Apn::Com_bytes::Picture_style)
            _set_conf(gp2::Conf_param::PICTURE_STYLE,r_data,t_data);
        else
        {
            t_data+=(char)Tram::Com_bytes::NAK;
            t_data+="demande de Set_Config inconnue: ";
            t_data+=r_data[2];
        }
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Capture_Eos_Dslr)
    {
        t_data+=(char)Tram::Com_bytes::ACK;
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Download)
    {
        t_data+=(char)Tram::Com_bytes::NAK;
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Delete_File)
    {
        _remove(r_data,t_data);
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Ls_Files)
    {
        t_data+=(char)Tram::Com_bytes::ACK;
    }
    else
    {
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+="commande inconnue: ";
        t_data+=r_data[1];
    }


    t_data+=(char)Tram::Com_bytes::EOT;

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
            Request=Read_Tram(Tram::Com_bytes::EOT,Server,Id_Socket,500);

            if(Request.size()<=0)
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
                 std::clog << "Tram R: ("<< Request.size() << " octets) " <<std::hex;
                for(auto & i:Request.get_data())
                    std::clog <<"0x"<< static_cast<int>(i) << " ";
                std::clog  << std::dec << std::endl<< std::endl;
            #endif // __DEBUG_MODE

            check_acknowledge(Request.get_c_data());

            process(Request.get_c_data(),Respond);

            #ifdef __DEBUG_MODE
                std::clog << "Tram T: ("<< Respond.size() << " octets) " <<std::hex;
                for(auto & i:Respond.get_data())
                    std::clog <<"0x"<< static_cast<int>(i) << " ";
                std::clog  << std::dec << std::endl<< std::endl;
            #endif // __DEBUG_MODE

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
