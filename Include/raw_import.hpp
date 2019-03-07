#ifndef RAW_IMPORT_HPP_INCLUDED
#define RAW_IMPORT_HPP_INCLUDED

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <libraw/libraw.h>
#include "Error.hpp"

class RAW
{
public:
    class Erreur : public Error
    {
    public:
        Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="gp2::Erreur";};
        virtual ~Erreur(){};
    };
///-------------------------------------------------------------
///initialisation valeur
///-------------------------------------------------------------
    RAW(void):m_file(""),m_data(nullptr),m_resolution(0,0)
    {
    }

    ~RAW()
    {
    }

    RAW(std::string const & file):m_file(file),m_data(nullptr),m_resolution(0,0)
    {
    }

///-------------------------------------------------------------
///constructeur de copie (poiteur partager sur data)
///-------------------------------------------------------------
    RAW(RAW const & cpy):m_file(cpy.get_file()),m_data(std::make_shared<cv::Mat>(cpy.get_data())),m_resolution(cpy.get_resolution())
    {
    }

///-------------------------------------------------------------
///operateur de copie (poiteur partager sur data)
///-------------------------------------------------------------
    RAW operator=(RAW const & cpy)
    {
        this->m_file=cpy.get_file();
        this->m_data=std::make_shared<cv::Mat>(cpy.get_data());

        this->m_resolution=cpy.get_resolution();

        return *this;
    }
///-------------------------------------------------------------
///renvoie le chemin de l'image
///-------------------------------------------------------------
    std::string get_file(void) const
    {
        return this->m_file;
    }

///-------------------------------------------------------------
///renvoie les données image const
///-------------------------------------------------------------
    cv::Mat get_data(void)const
    {
        return *this->m_data;
    }

///-------------------------------------------------------------
///renvoie les données image referencer
///-------------------------------------------------------------
    cv::Mat & data(void)
    {
        return *this->m_data;
    }

///-------------------------------------------------------------
///renvoie le pointeur des données image
///-------------------------------------------------------------
    std::shared_ptr<cv::Mat> get_data(void)
    {
        return this->m_data;
    }

///-------------------------------------------------------------
///import les image brut au format d'opencv2
///-------------------------------------------------------------
    bool load(void)
    {
        try
        {
            LibRaw lrProc;

            ///gestion des erreur de libraw
            if( LIBRAW_SUCCESS != lrProc.open_file( this->m_file.c_str()) )
                throw RAW::Erreur(1,"brute introuvable: "+this->m_file,RAW::Erreur::niveau::ERROR);
            if( LIBRAW_SUCCESS != lrProc.unpack())
                throw RAW::Erreur(2,"brute indecompressable: "+this->m_file,RAW::Erreur::niveau::ERROR);
            if( LIBRAW_SUCCESS != lrProc.raw2image() )
                throw RAW::Erreur(3,"brute impoossible a convertire en data: "+this->m_file,RAW::Erreur::niveau::ERROR);

            ///taille de l'image
            this->m_resolution.x = lrProc.imgdata.sizes.iwidth;
            this->m_resolution.y  = lrProc.imgdata.sizes.iheight;

            ///interpollation de la matrice de bayer
            std::vector<ushort> vBayerData;
            for ( int y = 0; y < this->m_resolution.y; y++ )
            {
                for ( int x = 0; x < this->m_resolution.x; x++ )
                {
                    int idx = y * this->m_resolution.x + x;

                    ushort * uRGBG = lrProc.imgdata.image[idx];

                    if ( y % 2 == 0 )
                    {
                        bool red = x % 2 == 0;
                        vBayerData.push_back( uRGBG[red ? 0 : 1] );
                    }
                    else
                    {
                        bool green = x % 2 == 0;
                        vBayerData.push_back( uRGBG[green ? 3 : 2] );
                    }
                }
            }

            ///convertion au format bgr d'opencv2 avec ajustement gamma

            lrProc.recycle();
            cv::Mat imgBayer( this->m_resolution.y, this->m_resolution.x, CV_16UC1, vBayerData.data() );

            cv::Mat imgDeBayer;
            cv::cvtColor( imgBayer, imgDeBayer, CV_BayerBG2BGR );

            float fGamma = 2.2f;
            imgDeBayer.convertTo( imgDeBayer, CV_32FC3, 1. / 4096);//4096-> 2^12 ( 12-Bit)
            cv::pow( imgDeBayer, fGamma, imgDeBayer );

            this->m_data=std::make_shared<cv::Mat>(imgDeBayer);

            return true;
        }
        catch(Error & e)
        {
            std::cerr << e.what() <<std::endl;

            ///image vide si erreur
            this->m_data=std::make_shared<cv::Mat>(cv::Mat::zeros(3000, 5000,  CV_32FC3));
            this->m_resolution=cv::Point(5000,3000);

            return false;
        }

        ///image vide si erreur
        this->m_data=std::make_shared<cv::Mat>(cv::Mat::zeros(3000, 5000,  CV_32FC3));
        this->m_resolution=cv::Point(5000,3000);

        return false;
    }

    cv::Point get_resolution() const
    {
        return this->m_resolution;
    }

private:
    std::string m_file;
    std::shared_ptr<cv::Mat> m_data;

    cv::Point m_resolution;
};

#endif // RAW_IMPORT_HPP_INCLUDED
