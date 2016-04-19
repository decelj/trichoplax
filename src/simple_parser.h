#pragma once

#include <string>
#include "iparser.h"

class Scene;

class SimpleParser : public IParser
{
public:
    SimpleParser();
    
    virtual std::string parse(const std::string& file, Scene& scene);
};
