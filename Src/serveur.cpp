#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include "tram.hpp"

class CSocketTCPServeur
{
    public:

    ///heritage de la class d'erreur
    class Erreur : public Error
    {
    public:
         Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="CSocketTCPServeur::Erreur";};
        virtual ~Erreur(){};
    };

    CSocketTCPServeur(void)
    {

    }

    ~CSocketTCPServeur(void)
    {
    }

    void NewSocket(unsigned int const &idx)
    {
        Sk_Channel[idx]=std::shared_ptr<SOCKET>(new SOCKET(socket(AF_INET,SOCK_STREAM,0)));

        if(*Sk_Channel[idx]==INVALID_SOCKET)
            throw CSocketTCPServeur::Erreur(1,"Socket Serveur erreur",Error::niveau::ERROR);
    }

    void CloseSocket(unsigned int const & idx)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Channel.find(idx);

        if(it!=Sk_Channel.end())
        {
            closesocket(*Sk_Channel[idx]);
            Sk_Channel.erase(it);
        }
    }

    void BindServeur(unsigned int const & idx,uint32_t const &addr,uint32_t const &port)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Channel.find(idx);

        if(it==Sk_Channel.end())
            throw CSocketTCPServeur::Erreur(2,"Socket ("+ ss_cast<unsigned int,std::string>(idx)+") inutilisable",Error::niveau::ERROR);

        this->ServerAdress.sin_addr.s_addr=addr;
        this->ServerAdress.sin_family=AF_INET;
        this->ServerAdress.sin_port=htons(port);

        if(bind(*Sk_Channel[idx],(struct sockaddr *)&ServerAdress,sizeof(ServerAdress))==SOCKET_ERROR)
            throw CSocketTCPServeur::Erreur(3,"le server n'a pas reussi a bind sur le port : "+ss_cast<uint32_t,std::string>(port),Error::niveau::ERROR);
    }
    void Listen(unsigned int const &idx,unsigned int const &nb)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Channel.find(idx);

        if(it==Sk_Channel.end())
            throw CSocketTCPServeur::Erreur(4,"Socket ("+ ss_cast<unsigned int,std::string>(idx)+") inutilisable",Error::niveau::ERROR);

        listen(*Sk_Channel[idx],nb);
    }

    void AcceptClient(unsigned int const &idx,unsigned int const &idxclient)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Channel.find(idx);

        if(it==Sk_Channel.end())
            throw CSocketTCPServeur::Erreur(5,"Socket ("+ ss_cast<unsigned int,std::string>(idx)+") inutilisable",Error::niveau::ERROR);

        int cu=sizeof(struct sockaddr_in);
        Sk_Client[idxclient]=std::shared_ptr<SOCKET>(new SOCKET(accept(*Sk_Channel[idx],(struct sockaddr *)&ClientAdress,(socklen_t *)&cu)));

        if(*Sk_Client[idxclient]<0)
            throw CSocketTCPServeur::Erreur(6,"la connection n'a pas pue etre établie",Error::niveau::ERROR);

    }

    void Write(unsigned int const &idx,VCHAR const &buffer)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Client.find(idx);

        if(it==Sk_Client.end())
            throw CSocketTCPServeur::Erreur(7,"Socket ("+ ss_cast<unsigned int,std::string>(idx)+") inutilisable",Error::niveau::ERROR);

            send(*Sk_Client[idx],buffer.data(),buffer.size(),0);
    }
    template<unsigned int octets>int Read(unsigned int const &idx,VCHAR &buffer)
    {
        std::map<unsigned int,std::shared_ptr<SOCKET>>::iterator it;
        it=Sk_Client.find(idx);
        char buf[octets];
        if(it==Sk_Client.end())
            throw CSocketTCPServeur::Erreur(8,"Socket ("+ ss_cast<unsigned int,std::string>(idx)+") inutilisable",Error::niveau::ERROR);

        int lenght= recv(*Sk_Client[idx] ,buf, octets,0);

        buffer.clear();
        for(int i=0;i<lenght;i++)
            buffer.push_back(buf[i]);
        return lenght;
    }

private:

    std::map<unsigned int,std::shared_ptr<SOCKET>> Sk_Channel;
    std::map<unsigned int,std::shared_ptr<SOCKET>> Sk_Client;
    SOCKADDR_IN ServerAdress,ClientAdress;
};

