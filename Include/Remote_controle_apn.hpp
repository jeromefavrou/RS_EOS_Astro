#ifndef REMOTE_CONTROLE_APN_HPP_INCLUDED
#define REMOTE_CONTROLE_APN_HPP_INCLUDED

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <sstream>
#include <memory>
#include "tram.hpp"
#include "client_tcp.hpp"
#include "gp2_utility.hpp"

class RC_Apn
{
public:

    ///defini si mode debug activé
    bool debug_mode;
    ///coupe coller
    bool download_and_remove;

    bool no_delete;
    bool no_download;

    ///passage des instruction a un serveur
    bool tcp_client;
    ///definit si mode de compatibilité avec ancien apn activé
    bool older;

    ///heritage de la class d'erreur
    class Erreur : public Error
    {
    public:
         Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="RC_Apn::Erreur";};
        virtual ~Erreur(){};
    };
    ///byts de communication
    class Com_bytes
    {
    public:
        static char constexpr Check_Apn=0x3A; //check apn
        static char constexpr Set_Config=0x3B; //set config
        static char constexpr Get_Config=0x3C; //get config
        static char constexpr Capture_Eos_Dslr=0x3D; //capture eos dslr
        static char constexpr Download=0x3E; //download
        static char constexpr Download_And_Remove=0x3F; //download and remove

        static char constexpr Aperture=0x61;
        static char constexpr Shutterspeed=0x62;
        static char constexpr Iso=0x63;
        static char constexpr Format=0x64;
        static char constexpr Target=0x65;
        static char constexpr White_balance=0x66;
        static char constexpr Picture_style=0x67;
        static char constexpr File=0x68;
        static char constexpr Older=0x69;
        static char constexpr Exposure=0x6A;
        static char constexpr Intervalle=0x6B;
        static char constexpr Debug_mode=0x6C;
        static char constexpr Tcp_client=0x6D;
    };
    ///type _data
    typedef gp2::Data _Data;

    RC_Apn(void);
    ~RC_Apn(void);
    void connect(struct t_connect const & tc);
    void init_conf_param(void);
    void check_apn(void);
    void capture_EOS_DSLR(bool setting,std::string inter,std::string iso,std::string exposure,std::string aperture,std::string target,std::string format, std::string shutter,std::string wb,std::string effect);

    void set_config(gp2::Conf_param const & param,std::string value);
    _Data & get_parameter(gp2::Conf_param const & param);
    char get_byte(gp2::Conf_param const & param);
    void download(gp2::Folder_data fd,std::string const & where);
    void delete_file(gp2::Folder_data fd);

    private:

    void download(std::string const &  why);
    void get_config(gp2::Conf_param const & param,_Data & gc);
    void check_acknowledge(VCHAR const & rep_tram);
    Tram Recv(int time_out);

    ///donné de sauvegarde parametre apn
    _Data m_aperture;
    _Data m_shutterspeed;
    _Data m_iso;
    _Data m_format;
    _Data m_target;
    _Data m_wb;
    _Data m_effect;
    _Data m_file;
    //si pas de donné ou erreur renvoie vide
    _Data m_void;

    //pour connection a serveur distant
    std::unique_ptr<CSocketTCPClient> m_client;
    int const m_id_client;
};

#endif // REMOTE_CONTROLE_APN_HPP_INCLUDED
