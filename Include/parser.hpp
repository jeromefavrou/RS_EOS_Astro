#ifndef PARSER_HPP_INCLUDED
#define PARSER_HPP_INCLUDED

#include <vector>
#include <algorithm>

///simple parametre unique pas d' options
namespace parser
{
    std::vector<std::string> parser(int argc,char ** argv)
    {
        std::vector<std::string> parsers;

        if(argc < 2) return parsers;//pas de parametre
        else
        {
            //lecture de tout les parametres
            for(auto i=1;i<argc;i++)//on ignore le parametre de nom d'executable
            {
                parsers.push_back(argv[i]);
            }
        }

        return parsers;
    }

    bool find(std::string const & p,std::vector<std::string> const & parser)
    {
         return std::find(parser.begin(), parser.end(), p)!=parser.end()?true:false;
    }
}


#endif // PARSER_HPP_INCLUDED
