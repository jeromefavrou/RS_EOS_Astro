#include "tram.hpp"

Tram::Tram(void){}
Tram::Tram(Tram const & cpy):m_data(cpy.get_c_data()){}
Tram::Tram(std::string const & init):m_data(VCHAR(init.begin(),init.end())){}
Tram::Tram(VCHAR const & init):m_data(init){}

Tram Tram::operator=(Tram const & cpy)
{
    this->m_data=cpy.get_c_data();
    return *this;
}
Tram Tram::operator=(std::string const & init)
{
    this->m_data=VCHAR(init.begin(),init.end());
    return *this;
}
Tram Tram::operator=(VCHAR const & init)
{
    this->m_data=init;
    return *this;
}

Tram Tram::operator+(Tram const & add)
{
    Tram t(*this);

    for(auto & i : add.get_c_data())
        t.get_data().push_back(i);

    return t;
}
Tram Tram::operator+(std::string const & add)
{
    Tram t(*this);

    for(auto & i : add)
        t.get_data().push_back(i);

    return t;
}
Tram Tram::operator+(VCHAR const & add)
{
    Tram t(*this);

    for(auto & i : add)
        t.get_data().push_back(i);

    return t;
}
Tram Tram::operator+(char const & add)
{
    Tram t(*this);

    t.get_data().push_back(add);

    return t;
}

Tram Tram::operator+=(Tram const & add)
{
    for(auto & i : add.get_c_data())
        this->m_data.push_back(i);

    return *this;
}
Tram Tram::operator+=(std::string const & add)
{
    for(auto & i : add)
        this->m_data.push_back(i);

    return *this;
}
Tram Tram::operator+=(VCHAR const & add)
{
    this->m_data.insert(this->m_data.end(),add.begin(),add.end());

    return *this;
}
Tram Tram::operator+=(char const & add)
{
    this->m_data.push_back(add);

    return *this;
}

void Tram::clear() noexcept
{
    m_data.clear();
}
size_t Tram::size() const noexcept
{
    return this->m_data.size();
}

VCHAR  & Tram::get_data(void)
{
    return this->m_data;
}

VCHAR Tram::get_c_data(void) const
{
    return this->m_data;
}
