#ifndef ERROR_HPP_INCLUDED
#define ERROR_HPP_INCLUDED

#include <exception>
#include <string>
#include "utility.hpp"

class Error
{
public:
    enum niveau{WARNING,ERROR,FATAL_ERROR};

    Error(int numero, std::string const& phrase,niveau _niveau) throw();
    std::string what() throw();
    niveau get_niveau(void) const;
    virtual ~Error() throw();

protected:

    int m_numero;               //Numéro de l'erreur
    std::string m_phrase;
    std::string m_class;
    niveau m_niveau;               //Niveau de l'erreur
};

#endif // ERROR_HPP_INCLUDED
