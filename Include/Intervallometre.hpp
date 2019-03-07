#ifndef INTERVALLOMETRE_HPP_INCLUDED
#define INTERVALLOMETRE_HPP_INCLUDED

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>

#include "Remote_controle_apn.hpp"
#include "gui.hpp"
#include "utility.hpp"

class Intervallometre
{
public:
    ///defini si mode debug activé
    bool debug_mode;
    bool delete_always;

    ///heritage de la class d'erreur
    class Erreur : public Error
    {
    public:
         Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="Intervallometre::Erreur";};
        virtual ~Erreur(){};
    };

    ///defini les parametre d'une sequance
    struct Sequance
    {

        bool user_local; //si la commande vien du programe ou concerne gphoto2
        bool wait;

        int frame;
        int exposure;

        float intervalle;
        float delay;

        std::string iso;
        std::string type_raw;
        std::string work_dir;
        std::string aperture;
        std::string target;
        std::string format;
        std::string shutter;
        std::string effect;
        std::string wb;
    };

    Intervallometre(void);

    void load(std::string const & file);

    void run_seq(RC_Apn & apn,std::string & last_capt);

///-------------------------------------------------------------
///taille de sequance
///-------------------------------------------------------------
    size_t size(void)const noexcept
    {
        return this->m_seq.size();
    }

///-------------------------------------------------------------
///verifie si la sequance possede des erreur
///-------------------------------------------------------------
    void check_sequance(RC_Apn & apn)
    {
        if(this->debug_mode)
            std::clog << "check de la sequance" << std::endl;

        bool check(true);
        int line(0);
        for(auto & i: this->m_seq)
        {
            //on determine la ligne en cause
            line++;

            std::stringstream ss_buff;
            std::string val;

            //on verifie les parametre d'ouveture
            if(i.aperture!="-1")
            {
                ss_buff <<i.aperture;
                ss_buff >>val;
                if(check)
                    check=this->check_cmd(apn,gp2::Conf_param::APERTURE,line,val);
                else
                    this->check_cmd(apn,gp2::Conf_param::APERTURE,line,val);
            }

            //on verifie les parametre du shutterspeed
            if(i.shutter!="-1")
            {
                ss_buff.clear();
                ss_buff <<i.shutter;
                ss_buff >>val;
                if(check)
                    check=this->check_cmd(apn,gp2::Conf_param::SHUTTERSPEED,line,val);
                else
                    this->check_cmd(apn,gp2::Conf_param::SHUTTERSPEED,line,val);
            }
            //on verifie les balance de blanc
            if(i.wb!="-1")
            {
                ss_buff.clear();
                ss_buff <<i.wb;
                ss_buff >>val;
                if(check)
                    check=this->check_cmd(apn,gp2::Conf_param::WHITE_BALANCE,line,val);
                else
                    this->check_cmd(apn,gp2::Conf_param::WHITE_BALANCE,line,val);
            }
            //on verifie les effect appliquer
            if(i.effect!="-1")
            {
                ss_buff.clear();
                ss_buff <<i.effect;
                ss_buff >>val;
                if(check)
                    check=this->check_cmd(apn,gp2::Conf_param::PICTURE_STYLE,line,val);
                else
                    this->check_cmd(apn,gp2::Conf_param::PICTURE_STYLE,line,val);
            }
            //on verifie les iso
            if(i.iso!="-1")
            {
                ss_buff.clear();
                ss_buff <<i.iso;
                ss_buff >>val;
                if(check)
                    check=this->check_cmd(apn,gp2::Conf_param::ISO,line,val);
                else
                    this->check_cmd(apn,gp2::Conf_param::ISO,line,val);
            }

            //on verifie le delay, frame et intervalle
            if(i.delay<=0 && i.delay!=-1)
            {
                if(this->debug_mode)
                    std::cerr << "ligne " << line << " erreur pour wait "<<i.delay<<" non pris en charge (<=0)"<<std::endl;
                check=false;
            }


            if(i.frame<=0 && i.frame!=-1)
            {
                if(this->debug_mode)
                    std::cerr << "ligne " << line << " erreur pour wait "<<i.delay<<" non pris en charge (<=0)"<<std::endl;
                check=false;
            }


            if(i.intervalle<1 && i.intervalle!=-1)
            {
                if(this->debug_mode)
                    std::cerr << "ligne " << line << " erreur pour wait "<<i.delay<<" non pris en charge (< 1)"<<std::endl;
                check=false;
            }

            if(i.type_raw != "dark" && i.type_raw != "flat" && i.type_raw != "offset" && i.type_raw != "light" && i.type_raw != "-1")
            {
                if(this->debug_mode)
                    std::cerr << "ligne " << line << " erreur pour TYPE_RAW "<<i.type_raw<<" non pris en charge (!= dark || offset || flat || light)"<<std::endl;
                check=false;
            }

            if(i.work_dir != "-1")
            {
                try
                {
                    std::ofstream Of(i.work_dir+"/test_dir");
                    if(!Of || Of.fail() || Of.bad())
                        throw Intervallometre::Erreur(1,"le repertoire de travail \""+i.work_dir+"\" n'a pas été trouvé",Intervallometre::Erreur::niveau::WARNING);
                }
                catch(Intervallometre::Erreur & e)
                {
                    std::cerr << e.what() << std::endl;
                    if(e.get_niveau()==Intervallometre::Erreur::niveau::WARNING)
                    {
                        free_cmd("mkdir -vp "+i.work_dir,this->debug_mode);
                        std::ofstream Of(i.work_dir+"/test_dir");
                        if(!Of || Of.fail() || Of.bad())
                        {
                            i.work_dir="-1";
                                throw Intervallometre::Erreur(2,"le repertoire de travail \""+i.work_dir+"\" est inutilisable",Intervallometre::Erreur::niveau::ERROR);
                        }
                    }
                }

                std::remove(std::string(i.work_dir+"/test_dir").c_str());
            }
        }
        if(!check)
            throw Intervallometre::Erreur(2,"Une ou plusieur erreur on été trouvées dans la sequance \"--debug-mode\" pour plus d'info ",Intervallometre::Erreur::ERROR);
    }

private:

///-------------------------------------------------------------
///on interprete chaque instruction
///-------------------------------------------------------------
    Sequance interpreter(std::stringstream  & ss_buffer)
    {
        //on initialise npar defaut a false ou -1
        Sequance seq;
        seq.wait=false;
        seq.type_raw="-1";
        seq.work_dir="-1";
        seq.delay=-1;
        seq.frame=-1;
        seq.intervalle=-1;
        seq.iso="-1";
        seq.aperture="-1";
        seq.exposure=-1;
        seq.shutter="-1";
        seq.effect="-1";
        seq.wb="-1";

        std::string cmd;

        //on interprete la sequance et les parametre associer
        while(ss_buffer)
        {
            ss_buffer >> cmd;

            if(cmd=="LOC") seq.user_local=true;
            else if(cmd=="GP2") seq.user_local=false;
            else if(cmd=="WAIT") seq.wait=true;
            else if(cmd=="DELAY") ss_buffer >> seq.delay;
            else if(cmd=="FRAME") ss_buffer >> seq.frame;
            else if(cmd=="INTER") ss_buffer >> seq.intervalle;
            else if(cmd=="ISO") ss_buffer >> seq.iso;
            else if(cmd=="EXPO") ss_buffer >> seq.exposure;
            else if(cmd=="APER") ss_buffer >> seq.aperture;
            else if(cmd=="TARGET") ss_buffer >> seq.target;
            else if(cmd=="FORMAT") ss_buffer >> seq.format;
            else if(cmd=="SHUTTER") ss_buffer >> seq.shutter;
            else if(cmd=="EFFECT") ss_buffer >> seq.effect;
            else if(cmd=="WB") ss_buffer >> seq.wb;
            else if(cmd=="WORK_DIRECTORY") ss_buffer >> seq.work_dir;
            else if(cmd=="TYPE_RAW") ss_buffer >> seq.type_raw;
            else
            {
                std::cout << "instruction: "<<cmd << " invalide donc ignoré" << std::endl;
            }
        }

        return seq;
    }

///-------------------------------------------------------------
///verifie si les instruction sont erroné
///-------------------------------------------------------------
    bool check_cmd(RC_Apn & apn,gp2::Conf_param const & param,int line,std::string value)
    {
        auto v= apn.get_parameter(param);

        auto res= std::find(v.begin(),v.end(),value);

        if(res == v.end())
        {
            //informe sur l'erreur
            if(this->debug_mode)
                std::cerr << "ligne " << line << " erreur pour " << gp2::Conf_param_to_str(param) <<" "<<value<<" non pris en charge"<<std::endl;

            return false;
        }
        else if(this->debug_mode)
            std::cout << "commande checker avec succes" <<std::endl;

        return true;
    }

    gp2::Folder_data delta_folder(gp2::Folder_data const & ref_f,gp2::Folder_data const & data_f)
    {
        gp2::Folder_data delta=data_f;
        bool f(false);

        for(auto F=delta.begin();F!=delta.end();F++)
        {
            for(auto G=F->second.begin();G!=F->second.end();G++)
            {
                 for(auto H=ref_f.begin();H!=ref_f.end();H++)
                 {
                    for(auto I=H->second.begin();I!=H->second.end();I++)
                    {
                        if(H->first==F->first && *G==*I)
                        {
                            F->second.erase(G);
                            G--;

                            H=ref_f.begin();
                            break;
                        }
                    }
                 }
            }
        }

        return delta;
    }

    //fichier pris par le programme
    gp2::Folder_data ref_file_capture;
    gp2::Folder_data file_capture;

    std::vector<Sequance> m_seq;
};

#endif // INTERVALLOMETRE_HPP_INCLUDED
