#ifndef UTILITY_HPP_INCLUDED
#define UTILITY_HPP_INCLUDED

#include <dirent.h>
#include <vector>
#include <string>
#include <exception>
#include <system_error>
#include <cstdlib>
#include <iostream>
#include <sstream>
enum unix_color{BLACK=30,RED=31,GREEN=32,YELLOW=33,BLUE=34,PINK=35,CYAN=36,GREY=37};

template<class T1,class T2> T2 ss_cast(T1 const & d)
{
    std::stringstream _ss_cast;

    _ss_cast << d;

    T2 tps;

    _ss_cast >> tps;

    return tps;
}

template<unix_color uc> std::string _print(std::string const & msg)
{
    std::string nwmsg("");

    #ifndef WIN32
    nwmsg+="\033[";
    nwmsg+=ss_cast<unsigned int,std::string>(static_cast<unsigned int>(uc));
    nwmsg+="m";
    nwmsg+=msg;
    nwmsg+="\033[0m";
    #else
    nwmsg=msg;
    #endif // WIN32

    return nwmsg;
}

std::vector<std::string> ls(std::string const & file)noexcept;
void notify_send(std::string const & msg);
void free_cmd(std::string const & cmd,bool debug_mode);


#endif // UTILITY_HPP_INCLUDED
