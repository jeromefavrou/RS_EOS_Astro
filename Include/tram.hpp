#ifndef TRAM_HPP_INCLUDED
#define TRAM_HPP_INCLUDED

#include <vector>
#include <string>
#include "Error.hpp"

typedef std::vector<char> VCHAR;

class Tram
{
    public:

        class Erreur : public Error
        {
        public:
            Erreur(int numero, std::string const& phrase,niveau _niveau)throw():Error(numero,phrase,_niveau){this->m_class="gp2::Erreur";};
            virtual ~Erreur(){};
        };
        //ascii correspondance
        class Com_bytes
        {
        public:
            static char constexpr SOH=0x01; //Start of Heading
            static char constexpr EOT=0x04; //End of Transmission
            static char constexpr ACK=0x06; //Acknowledge
            static char constexpr NAK=0x15; //Negative Acknowledge
            static char constexpr US= 0x1F; //Unit Separator
            static char constexpr GS= 0x1D; //Group Separator
        };

        Tram(void);
        Tram(Tram const &);
        Tram(std::string const &);
        Tram(VCHAR const &);

        Tram operator=(Tram const & );
        Tram operator=(std::string const &);
        Tram operator=(VCHAR const &);

        Tram operator+(Tram const &);
        Tram operator+(std::string const &);
        Tram operator+(VCHAR const &);
        Tram operator+(char const &);

        Tram operator+=(Tram const &);
        Tram operator+=(std::string const &);
        Tram operator+=(VCHAR const &);
        Tram operator+=(char const &);

        void clear() noexcept;
        size_t size() const noexcept;

        VCHAR  & get_data(void);

        VCHAR get_c_data(void) const;

    private:

        VCHAR m_data;

};

#endif // TRAM_HPP_INCLUDED
