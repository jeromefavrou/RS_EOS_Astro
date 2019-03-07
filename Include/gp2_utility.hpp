#ifndef GP2_UTILITY_HPP_INCLUDED
#define GP2_UTILITY_HPP_INCLUDED

#include <fstream>
#include <memory>
#include <map>
#include "utility.hpp"
#include "Error.hpp"

namespace gp2
{
    typedef std::vector<std::string> Data;
    typedef std::map<std::string,Data> Folder_data;

    class Erreur : public Error
    {
    public:
        Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="gp2::Erreur";};
        virtual ~Erreur(){};
    };

    enum Conf_param{ APERTURE=1, ISO , SHUTTERSPEED , FORMAT , TARGET , WHITE_BALANCE , PICTURE_STYLE};
    enum Gest_file_param{LS_FILES=8,LS_FOLDER,RMDIR,MKDIR,GET_FILE,DELETE_FILE};

    struct mnt
    {
        std::string cmd;
        std::string path;
    };

    void Mount(struct mnt const & ifm);
    void Unmount(struct mnt const & ifm);

    std::string Conf_param_to_str(Conf_param const & param);
    void Get_config(Conf_param const & param,Data & gc);
    void Set_config(Conf_param const & param,std::string value,bool debug_mode);
    void List_files(Folder_data & F_data,bool debug_mode);
    void Auto_detect(Data & device);
    void Delete_file(std::string const & file,bool debug_mode);
}


#endif // GP2_UTILITY_HPP_INCLUDED
