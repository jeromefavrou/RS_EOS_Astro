//#define __DEBUG_MODE

#define MNT_CONF_PATH ".mnt_configure"
#define SERVER_CONF_PATH ".server_configure"

#define Id_Socket 0

#include "serveur.cpp"
#include "gp2_utility.hpp"
#include <chrono>
#include <thread>

namespace RC_Apn
{
    class Com_bytes
{
public:
	static char constexpr Check_Apn=0x3A; //check apn
        static char constexpr Set_Config=0x3B; //set config
        static char constexpr Get_Config=0x3C; //get config
        static char constexpr Capture_Eos_Dslr=0x3D; //capture eos dslr
        static char constexpr Download=0x3E; //download
        static char constexpr Delete_File=0x3F; //download and remove
        static char constexpr Ls_Files=0x40; //listing file
        static char constexpr Mk_dir=0x41; //mk_dir
        static char constexpr Check_Mem=0x41; //check free memory

        static char constexpr Aperture=0x61;
        static char constexpr Shutterspeed=0x62;
        static char constexpr Iso=0x63;
        static char constexpr Format=0x64;
        static char constexpr Target=0x65;
        static char constexpr White_balance=0x66;
        static char constexpr Picture_style=0x67;
        static char constexpr Older=0x69;
        static char constexpr Exposure=0x6A;
        static char constexpr Intervalle=0x6B;

        static char constexpr Debug_mode=0x6C;
        static char constexpr Tcp_client=0x6D;
};
}


Tram Read_Tram(char const ending_byte,CSocketTCPServeur & Server,int id_server,int const _time_out)
{
    Tram data;

    bool Do=true;


    while(Do)
    {
        VCHAR tps;

        Server.Read<2048>(id_server,tps);

        if(tps.size()<=0)
            throw Error(1,"client deconnecté",Error::niveau::WARNING);

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
    std::cout <<"(get_config)["+gp2::Conf_param_to_str(cp)+"]"<< std::endl;
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

        t_data.clear();
        t_data+=(char)Tram::Com_bytes::SOH;
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void _set_conf(gp2::Conf_param const& cp,VCHAR const & r_data,Tram & t_data)
{
    std::cout <<"(set_config)["+gp2::Conf_param_to_str(cp)+"]"<< std::endl;
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

        t_data.clear();
        t_data+=(char)Tram::Com_bytes::SOH;
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

        t_data.clear();
        t_data+=(char)Tram::Com_bytes::SOH;
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void _ls_file(Tram & t_data)
{
    try
    {
        gp2::Folder_data fd;
        gp2::List_files(fd,true);

        t_data+=(char)Tram::Com_bytes::ACK;

        for(auto file=fd.begin();file!=fd.end();file++)
        {

            t_data+=std::string(file->first);
            t_data+=(char)Tram::Com_bytes::GS;

            for(auto name=file->second.begin();name!=file->second.end();name++)
            {
                t_data+=std::string(*name);

                t_data+=(char)Tram::Com_bytes::US;
            }
        }
    }
    catch(Error & e)
    {
        std::cerr<< e.what() <<std::endl;

        t_data.clear();
        t_data+=(char)Tram::Com_bytes::SOH;
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void _capture(VCHAR const & r_data,Tram & t_data)
{
    try
    {
        std::string exposure("");

        for(auto i=2;i<r_data.size();i++)
        {
            if(r_data[i]==(char)Tram::Com_bytes::EOT)
                break;
            else if(r_data[i]==(char)Tram::Com_bytes::US)
                break;

            exposure+=r_data[i];
        }

        gp2::Capture(exposure,false);

        t_data+=(char)Tram::Com_bytes::ACK;
    }
    catch(Error & e)
    {
        std::cerr<< e.what() <<std::endl;

        t_data.clear();
        t_data+=(char)Tram::Com_bytes::SOH;
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
    }
}

void _download(VCHAR const & r_data,Tram & t_data)
{
    try
    {
        std::string folder(""),file(""),buff("");

        for(auto i=2;i<r_data.size();i++)
        {
            if(r_data[i]==Tram::Com_bytes::EOT)
                break;

            else if(r_data[i]==Tram::Com_bytes::GS)
            {
                folder=buff;
                buff="";
                continue;
            }
            else if(r_data[i]==Tram::Com_bytes::US)
            {
                file=buff;
                buff="";
                continue;
            }

            buff+=r_data[i];
        }

        gp2::Download_file(folder+"/"+file,false);

        t_data+=(char)Tram::Com_bytes::ACK;
    }
    catch(Error & e)
    {
        std::cerr<< e.what() <<std::endl;

        t_data.clear();
        t_data+=(char)Tram::Com_bytes::SOH;
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+=e.str();
        t_data+=(char)Tram::Com_bytes::EOT;
    }
}

void process(VCHAR const & r_data,Tram & t_data,bool & serv_b)
{
    struct gp2::mnt _mnt{"gio mount",""};
    t_data.clear();
    t_data+=(char)Tram::Com_bytes::SOH;

    if(r_data[1]==(char)RC_Apn::Com_bytes::Check_Apn)
    {
        std::cout <<"(check apn)"<< std::endl;
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
        std::cout <<"(Older)"<< std::endl;
        t_data+=(char)Tram::Com_bytes::ACK;
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Debug_mode)
    {
        std::cout <<"(Debug_mode)"<< std::endl;
        t_data+=(char)Tram::Com_bytes::ACK;
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Tcp_client)
    {
        std::cout <<"(Tcp_client)"<< std::endl;
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
        std::cout <<"(Capture)"<< std::endl;
        _capture(r_data,t_data);
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Download)
    {
        std::cout <<"(Download_file)"<< std::endl;
        _download(r_data,t_data);
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Delete_File)
    {
        std::cout <<"(Delete_file)"<< std::endl;
        _remove(r_data,t_data);
    }
    else if(r_data[1]==(char)RC_Apn::Com_bytes::Ls_Files)
    {
        std::cout <<"(Ls_files)"<< std::endl;
        _ls_file(t_data);
    }
    else if(r_data[1]==(char)Tram::Com_bytes::CS)
    {
        serv_b=false;
    }
    else
    {
        std::cout <<"(non reconnue)"<< std::endl;
        t_data+=(char)Tram::Com_bytes::NAK;
        t_data+="commande inconnue: ";
        t_data+=r_data[1];
    }

    t_data+=(char)Tram::Com_bytes::EOT;
}

void init_mnt_configure(struct gp2::mnt & _mount)
{
    std::fstream If(MNT_CONF_PATH,std::ios::in);

    if(!If || If.bad() || If.fail())
        throw Error(1,"erreur a la lecture de \""+std::string(MNT_CONF_PATH)+"\"",Error::niveau::WARNING);

    std::getline(If,_mount.cmd);
    std::getline(If,_mount.path);
}

void init_server_configure(uint32_t & port)
{
    std::fstream If(SERVER_CONF_PATH,std::ios::in);

    if(!If || If.bad() || If.fail())
        throw Error(1,"erreur a la lecture de \""+std::string(SERVER_CONF_PATH)+"\"",Error::niveau::WARNING);

    If >> port;
}

int main(int argc,char ** argv)
{
    struct gp2::mnt _mount;

    bool as_client=false;

    CSocketTCPServeur Server;

    try
    {
        if(system(nullptr));
        else
            throw Error(1,"les commandes system ne peuvent etre utilise",Error::niveau::FATAL_ERROR);

        init_mnt_configure(_mount);
    }
    catch(Error & e)
    {
        std::cerr << e.what() << std::endl;

        if(e.get_niveau()==Error::niveau::FATAL_ERROR)
            return -1;

        #ifndef WIN32
        _mount.cmd="gio mount";
        _mount.path="/run/user/1000/gvfs";
        #elif
        return -1;
        #endif // WIN32
    }

    uint32_t port;
    try
    {
        init_server_configure(port);
    }
    catch(Error & e)
    {
        std::cerr << e.what() << std::endl;

        if(e.get_niveau()==Error::niveau::FATAL_ERROR)
            return -1;

        port=6789;
    }
    try
    {
        Server.NewSocket(Id_Socket);
        Server.BindServeur(Id_Socket,INADDR_ANY,port);
        Server.Listen(Id_Socket,1);


        std::clog << "en attente de client" << std::endl;

        Server.AcceptClient(Id_Socket,0);

        std::clog << "client connecté" << std::endl;

        as_client=true;
    }
    catch(Error & e)
    {
        std::cerr << e.what() << std::endl;

        if(e.get_niveau()!=Error::niveau::WARNING)
            return -1;
    }
    bool ser_b=true;
    while(ser_b)
    {
        Tram Request;
        Tram Respond;
        try
        {
            Request=Read_Tram(Tram::Com_bytes::EOT,Server,Id_Socket,500);
        }
        catch(Error & e)
        {
            std::cerr << e.what() << std::endl;

            if(e.get_niveau()==Error::niveau::FATAL_ERROR)
                return -1;


            std::clog << "attente de reconnexion du client" << std::endl;

            Server.AcceptClient(Id_Socket,0);

            std::clog << "client reconnecté" << std::endl;

            continue;
        }

        try
        {
            #ifdef __DEBUG_MODE
                 std::clog << "Tram R: ("<< Request.size() << " octets) " <<std::hex;
                for(auto & i:Request.get_data())
                    std::clog <<"0x"<< static_cast<int>(i) << " ";
                std::clog  << std::dec << std::endl;
            #endif // __DEBUG_MODE

            check_acknowledge(Request.get_c_data());

            process(Request.get_c_data(),Respond,ser_b);

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

            if(e.get_niveau()==Error::niveau::FATAL_ERROR)
                return -1;
        }

        std::this_thread::sleep_for(std::chrono::duration<int,std::milli>(20));
    }

    Server.CloseSocket(Id_Socket);

    return 0;
}
