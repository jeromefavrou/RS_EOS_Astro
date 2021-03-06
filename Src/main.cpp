//#define __DEBUG_MODE
#define VERSION "V_01.01"

#define MNT_CONF_PATH ".mnt_configure"
#define SERVER_CONF_PATH ".server_configure"

#define DEFAULT_PORT 6789

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

            static char constexpr Aperture=0x61;
            static char constexpr Shutterspeed=0x62;
            static char constexpr Iso=0x63;
            static char constexpr Format=0x64;
            static char constexpr Target=0x65;
            static char constexpr White_balance=0x66;
            static char constexpr Picture_style=0x67;
            static char constexpr Older=0x68;
            static char constexpr Exposure=0x69;
            static char constexpr Intervalle=0x6A;

            static char constexpr Debug_mode=0x6B;
            static char constexpr Tcp_client=0x6C;
    };
}

Tram Read_Tram(char const ending_byte,CSocketTCPServeur & Server,int id_server);

void check_acknowledge(VCHAR const & rep_tram);

void _get_conf(gp2::Conf_param const& cp,Tram & t_data,gp2::Data & gc);

void _set_conf(gp2::Conf_param const& cp,VCHAR const & r_data,Tram & t_data);

void _remove(VCHAR const & r_data,Tram & t_data);

void _ls_file(Tram & t_data);

void _capture(VCHAR const & r_data,Tram & t_data);

void _download(VCHAR const & r_data,Tram & t_data);

void _Mk_dir(VCHAR const & r_data,Tram & t_data);

void process(VCHAR const & r_data,Tram & t_data,bool & serv_b);

void init_mnt_configure(struct gp2::mnt & _mount);

void init_server_configure(uint32_t & port);

int main()
{
    struct gp2::mnt _mount;

    CSocketTCPServeur Server;

    try
    {
        if(system(nullptr));
        else
            throw Error(7,"les commandes system ne peuvent etre utilise",Error::niveau::FATAL_ERROR);

        init_mnt_configure(_mount);
    }
    catch(Error & e)
    {
        std::cerr << e.what() << std::endl;

        if(e.get_niveau()==Error::niveau::FATAL_ERROR)
            return -1;

        #ifndef WIN32
        _mount.cmd=DEFAULT_MNT_CMD;
        _mount.path=DEFAULT_MNT_FOLDER;
        #elif
        return -1;
        #endif // WIN32

        std::fstream Of(MNT_CONF_PATH,std::ios::out);

        Of << DEFAULT_MNT_CMD << std::endl;
        Of << DEFAULT_MNT_FOLDER;

        Of.close();
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

        port=DEFAULT_PORT;

        std::fstream Of(SERVER_CONF_PATH,std::ios::out);

        Of << DEFAULT_PORT;

        Of.close();

    }
    try
    {
        Server.NewSocket(Id_Socket);
        Server.BindServeur(Id_Socket,INADDR_ANY,port);
        Server.Listen(Id_Socket,1);


        std::cout << "en attente de client" << std::endl;

        Server.AcceptClient(Id_Socket,0);

        std::clog << "client connecté" << std::endl;
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
            Request=Read_Tram(Tram::Com_bytes::EOT,Server,Id_Socket);
        }
        catch(Error & e)
        {
            std::cerr << e.what() << std::endl;

            if(e.get_niveau()==Error::niveau::FATAL_ERROR)
            {
                Server.CloseSocket(Id_Socket);
                return -1;
            }

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
            {
                Server.CloseSocket(Id_Socket);
                return -1;
            }

        }

        std::this_thread::sleep_for(std::chrono::duration<int,std::milli>(50));
    }

    Server.CloseSocket(Id_Socket);

    return 0;
}

Tram Read_Tram(char const ending_byte,CSocketTCPServeur & Server,int id_server)
{
    Tram data;

    bool Do=true;
    short int nb_tent=0;

    while(Do)
    {
        VCHAR tps;

        Server.Read<2048>(id_server,tps);

        nb_tent++;

        if(tps.size()<=0)
            throw Error(0,"client deconnecté",Error::niveau::WARNING);

        if(nb_tent>=5)
            throw Error(1,"le client n'est pas compris par le server",Error::niveau::ERROR);

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

                throw Error(2,"erreur du serveur: "+er,Error::niveau::WARNING);
            }
            else
                return ;
        }
        else
            throw Error(3,"header et footer tram non respecté par le serveur: ",Error::niveau::ERROR);

    throw Error(4,"Erreur inconnue du avec le serveur: ",Error::niveau::ERROR);
}

void _get_conf(gp2::Conf_param const& cp,Tram & t_data,gp2::Data & gc)
{
    std::cout <<"(get_config)["+gp2::Conf_param_to_str(cp)+"]"<< std::endl;

    gp2::Get_config(cp,gc);

    t_data+=(char)Tram::Com_bytes::ACK;

    for(auto & i :gc)
    {
        t_data+=i;
        t_data+=(char)Tram::Com_bytes::US;
    }
}

void _set_conf(gp2::Conf_param const& cp,VCHAR const & r_data,Tram & t_data)
{
    std::cout <<"(set_config)["+gp2::Conf_param_to_str(cp)+"]"<< std::endl;

    std::string value("");

    for(auto i=3u;i<r_data.size();i++)
    {
        if(r_data[i]==Tram::Com_bytes::EOT)
            break;

        value+=(char)r_data[i];
    }

    gp2::Set_config(cp,value,false);

    t_data+=(char)Tram::Com_bytes::ACK;
}

void _remove(VCHAR const & r_data,Tram & t_data)
{
    std::string value("");

    for(auto i=2u;i<r_data.size();i++)
    {
        if(r_data[i]==Tram::Com_bytes::EOT)
            break;

        value+=(char)r_data[i];
    }

    gp2::Delete_file(value,false);

    t_data+=(char)Tram::Com_bytes::ACK;
}

void _ls_file(Tram & t_data)
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

void _capture(VCHAR const & r_data,Tram & t_data)
{
    std::string exposure("");

    for(auto i=2u;i<r_data.size();i++)
    {
        if(r_data[i]==(char)Tram::Com_bytes::EOT)
            break;
        exposure+=r_data[i];
    }

    gp2::Capture(exposure,false);

    t_data+=(char)Tram::Com_bytes::ACK;
}

void _download(VCHAR const & r_data,Tram & t_data)
{
    std::vector<std::string> parser;
    std::string buff("");

    for(auto i=2u;i<r_data.size();i++)
    {
        if(r_data[i]==Tram::Com_bytes::EOT)
            break;

        else if(r_data[i]==Tram::Com_bytes::US)
        {
            parser.push_back(buff);
            buff="";
            continue;
        }

        buff+=r_data[i];
    }

    if(parser.size()!=3)
        throw Error(5,"nombre de parametres pour download != 3",Error::niveau::ERROR);

    gp2::Download_file(parser[0]+"/"+parser[1],false);

    free_cmd("mv "+parser[1]+" "+parser[2],false);

    t_data+=(char)Tram::Com_bytes::ACK;
}

void _Mk_dir(VCHAR const & r_data,Tram & t_data)
{
    std::string parser("");

    for(auto i=2u;i<r_data.size();i++)
    {
        if(r_data[i]==Tram::Com_bytes::EOT)
            break;

        parser+=r_data[i];
    }

    free_cmd("mkdir -vp "+parser,false);

    t_data+=(char)Tram::Com_bytes::ACK;
}

void process(VCHAR const & r_data,Tram & t_data,bool & serv_b)
{
    struct gp2::mnt _mnt{"gio mount",""};
    t_data.clear();
    t_data+=(char)Tram::Com_bytes::SOH;

    try
    {
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
                _get_conf(gp2::Conf_param::APERTURE,t_data,gc);
            else if(r_data[2]==RC_Apn::Com_bytes::Shutterspeed)
                _get_conf(gp2::Conf_param::SHUTTERSPEED,t_data,gc);
            else if(r_data[2]==RC_Apn::Com_bytes::Iso)
                _get_conf(gp2::Conf_param::ISO,t_data,gc);
            else if(r_data[2]==RC_Apn::Com_bytes::Format)
                _get_conf(gp2::Conf_param::FORMAT,t_data,gc);
            else if(r_data[2]==RC_Apn::Com_bytes::Target)
                _get_conf(gp2::Conf_param::TARGET,t_data,gc);
            else if(r_data[2]==RC_Apn::Com_bytes::White_balance)
                _get_conf(gp2::Conf_param::WHITE_BALANCE,t_data,gc);
            else if(r_data[2]==RC_Apn::Com_bytes::Picture_style)
                _get_conf(gp2::Conf_param::PICTURE_STYLE,t_data,gc);
            else
                throw Error(6,"demande de Get_Config inconnue: "+r_data[2],Error::niveau::ERROR);

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
                throw Error(6,"demande de Set_Config inconnue: "+r_data[2],Error::niveau::ERROR);

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
        else if(r_data[1]==(char)RC_Apn::Com_bytes::Mk_dir)
        {
            std::cout <<"(Ls_files)"<< std::endl;
            _Mk_dir(r_data,t_data);
        }
        else if(r_data[1]==(char)Tram::Com_bytes::CS)
        {
            serv_b=false;
        }
        else if(r_data[1]==(char)Tram::Com_bytes::ENQ)
        {
            t_data+=(char)Tram::Com_bytes::ACK;
            t_data+=std::string(VERSION);
        }
        else
        {
            std::cout <<"(non reconnue)"<< std::endl;
            throw Error(6,"commande inconnue: "+r_data[1],Error::niveau::ERROR);
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
