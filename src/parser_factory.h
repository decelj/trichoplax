#ifndef __trichoplax__parser_factory__
#define __trichoplax__parser_factory__

#include <string>

#include "iparser.h"

class ParserFactory
{
public:
    ParserFactory() { }
    
    IParser* create(const std::string &file);
};

#endif /* defined(__trichoplax__parser_factory__) */
