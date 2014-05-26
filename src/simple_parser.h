#ifndef __trichoplax__simple_parser__
#define __trichoplax__simple_parser__

#include <string>

#include "iparser.h"

class Scene;

class SimpleParser : public IParser
{
public:
    SimpleParser() { }
    
    virtual std::string parse(const std::string& file, Scene* const scene);
};

#endif /* defined(__trichoplax__simple_parser__) */
