#include "Error.hpp"
#include <iostream>
Error::Error(int numero, std::string const& phrase,niveau _niveau) throw():m_numero(numero),m_phrase(phrase),m_niveau(_niveau)
{
    this->m_class="Error";
}
std::string Error::what() throw()
{
    std::string str_erreur("");
    std::string str("");

    switch(this->m_niveau)
    {
    case WARNING: str=_print<unix_color::GREEN>("WARNING"); break;
    case ERROR: str=_print<unix_color::BLUE>("ERROR"); break;
    case FATAL_ERROR: str=_print<unix_color::RED>("FATAL ERROR"); break;
    default : str="WARNING";
    }

    str_erreur="("+this->m_class+") ";
    str_erreur+=str+" [";
    str_erreur+=ss_cast<int,std::string>(this->m_numero)+"]: "+this->m_phrase;

    return str_erreur;
}
std::string Error::str(void) const
{
    return this->m_phrase;
}
Error::niveau Error::get_niveau(void) const
{
    return this->m_niveau;
}
Error::~Error() throw(){}
