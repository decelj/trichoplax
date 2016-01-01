#ifndef trichoplax_iparser_h
#define trichoplax_iparser_h

#include <string>

class Scene;

class IParser
{
public:
    IParser() { }
    virtual ~IParser() { }
    
    virtual std::string parse(const std::string& file, Scene& scene) = 0;
};

#endif
