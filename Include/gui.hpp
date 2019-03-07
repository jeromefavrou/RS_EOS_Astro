#ifndef GNU_HPP_INCLUDED
#define GNU_HPP_INCLUDED

#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include "raw_import.hpp"

class GUI
{
 public:
     bool debug_mode;

    class Erreur : public Error
    {
     public:
        Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="gp2::Erreur";};
        virtual ~Erreur(){};
    };

     GUI(void):m_name_blue_windows("BLUE"),m_name_green_windows("GREEN"),m_name_red_windows("RED"),m_name_full_windows("RGB")
     {
         this->debug_mode=false;
     }

     void raw_display(std::string & capt)
     {
        std::string t_capt("");
        int luminosite(50),contraste(50);

        cv::namedWindow(m_name_red_windows,cv::WINDOW_FREERATIO);
        cv::namedWindow(m_name_blue_windows,cv::WINDOW_FREERATIO);
        cv::namedWindow(m_name_green_windows,cv::WINDOW_FREERATIO);
        cv::namedWindow(m_name_full_windows,cv::WINDOW_FREERATIO);

        cv::resizeWindow(m_name_red_windows, 467, 350);
        cv::resizeWindow(m_name_blue_windows, 467, 350);
        cv::resizeWindow(m_name_green_windows, 467, 350);
        cv::resizeWindow(m_name_full_windows, 640, 480);

        cv::createTrackbar("luminosité", m_name_full_windows,&luminosite,100);
        cv::createTrackbar("contraste", m_name_full_windows,&contraste,100);


        while(capt!="exit")
        {
            RAW image(capt);
            image.load();

            int radius=image.get_resolution().x/16;

            cv::circle(image.data(),image.get_resolution()/2,radius,cv::Scalar( 0, 0, 255 ),7,cv::LINE_AA);
            cv::line(image.data(), image.get_resolution()/2+cv::Point(0,radius/5), image.get_resolution()/2+cv::Point(0,radius), cv::Scalar( 255, 0, 0 ), 5, cv::LINE_AA);
            cv::line(image.data(), image.get_resolution()/2+cv::Point(radius/5,0), image.get_resolution()/2+cv::Point(radius,0), cv::Scalar( 0, 255, 0 ), 5, cv::LINE_AA);

            std::vector<cv::Mat> rgbChannels(3);

            while(capt==t_capt && capt!="exit")
            {
                cv::Mat dst;

                image.data().convertTo(dst, -1, static_cast<float>(contraste)/50, static_cast<float>(luminosite)/10-5);

                cv::split(dst, rgbChannels);

                cv::imshow( m_name_red_windows, rgbChannels[2]);
                cv::imshow( m_name_blue_windows, rgbChannels[0]);
                cv::imshow( m_name_green_windows, rgbChannels[1]);
                cv::imshow( m_name_full_windows, dst);

                cv::waitKey(900);
            }

            t_capt=capt;
        }

        cv::destroyAllWindows();
     }
     void cam_display(int cam)
     {
        cv::VideoCapture cap(cam);

        //cap.set(cv::CAP_PROP_EXPOSURE,0.1);
        //cap.set(cv::CAP_PROP_GAIN,0.1);

        if(!cap.isOpened())
        {
            std::cout << "la camera n'a pas pu etre ouverte"<<std::endl;
            return;
        }

        cv::Mat Master;
        cv::Mat calib;

        std::cout << "calibration en cour caché la caméra dans le noir (bouchon, autre)" << std::endl;

        for(auto i=0;i<25;i++)
        {

            cap >> calib;
            if(i>0)
                Master=Master+calib;
            else
                Master=calib;
        }

        std::cout<<"calibration fini"<<std::endl;

        cv::Mat frame,frame1,dst;
        std::vector<cv::Mat> rgb;

        int debruitage(1);
        int calibration(1),t_calibration(1);
        int luminosite(50),contraste(50);

        cv::namedWindow(m_name_red_windows,cv::WINDOW_FREERATIO);
        cv::namedWindow(m_name_blue_windows,cv::WINDOW_FREERATIO);
        cv::namedWindow(m_name_green_windows,cv::WINDOW_FREERATIO);
        cv::namedWindow(m_name_full_windows,cv::WINDOW_FREERATIO);

        cv::resizeWindow(m_name_red_windows, 467, 350);
        cv::resizeWindow(m_name_blue_windows, 467, 350);
        cv::resizeWindow(m_name_green_windows, 467, 350);
        cv::resizeWindow(m_name_full_windows, 640, 480);

        cv::createTrackbar("debruitage",m_name_full_windows,&debruitage,4);
        cv::createTrackbar("luminosité", m_name_full_windows,&luminosite,100);
        cv::createTrackbar("contraste", m_name_full_windows,&contraste,100);
        cv::createTrackbar("intensité calib", m_name_full_windows,&calibration,24);

        for(;;)
        {
            if(debruitage==0)
                debruitage=1;

            t_calibration=25-calibration;

            cap >> frame;

            if(calibration!=0)
                frame=frame-(Master/t_calibration);

            for(auto i=0;i<debruitage-1;i++)
            {
                cap >> frame1;

                if(calibration!=0)
                    frame1=frame1-(Master/t_calibration);

                frame=frame+frame1;
            }

            frame=frame/(debruitage);

            frame.convertTo(dst, -1, static_cast<float>(contraste)/50, static_cast<float>(luminosite)-50);

            cv::split(dst,rgb);

            cv::imshow(m_name_full_windows, dst);

            cv::imshow(m_name_blue_windows, rgb[0]);
            cv::imshow(m_name_red_windows, rgb[2]);
            cv::imshow(m_name_green_windows, rgb[1]);

            if(cv::waitKey(20) >= 0) break;
        }
     }

 private:

     std::string const m_name_red_windows;
     std::string const m_name_green_windows;
     std::string const m_name_blue_windows;
     std::string const m_name_full_windows;
};

#endif // GNU_HPP_INCLUDED
