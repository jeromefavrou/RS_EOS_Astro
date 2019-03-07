#ifndef CLIENT_TCP_HPP_INCLUDED
#define CLIENT_TCP_HPP_INCLUDED

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket(s) close(s)

#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include "Error.hpp"
#include "tram.hpp"
#include "utility.hpp"

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

struct t_connect
{
    std::string addr;
    uint32_t port;
};

class CSocketTCPClient
{
    public:
    enum typeip{HOSTNAME,IP};
    bool debug_mode;

    ///heritage de la class d'erreur
    class Erreur : public Error
    {
    public:
         Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="CSocketTCPClient::Erreur";};
        virtual ~Erreur(){};
    };

    CSocketTCPClient(void);

    ~CSocketTCPClient(void);

    void NewSocket(unsigned int const &idx);

    void CloseSocket(unsigned int const & idx);

    void Connect(unsigned int const & idx,struct t_connect const & tc,typeip const & tip);

    void Write(unsigned int const &idx,VCHAR const & buffer);

    template<unsigned int octets>int Read(unsigned int const &idx,VCHAR &buffer)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Channel.find(idx);
        char buf[octets];
        if(it==Sk_Channel.end())
            throw std::range_error("cannot use a Socket not open");

        int lenght= recv(*Sk_Channel[idx] ,buf,octets,0);
        for(auto i=0;i<lenght;i++)
            buffer.push_back(buf[i]);
        return lenght;
    }

private:

    std::map<unsigned int,std::shared_ptr<SOCKET>> Sk_Channel;
    SOCKADDR_IN ServerAdress;
};

void ctrl_time_out(bool & stat,int const elaps_time);

Tram Read_Tram(char const ending_byte,CSocketTCPClient & Client,int id_client,int const _time_out);

#endif // CLIENT_TCP_HPP_INCLUDED
