#include <string>

#include "parser_factory.h"
#include "simple_parser.h"
#include "assimp_parser.h"

IParser* ParserFactory::create(const std::string &file)
{
    if (file.compare(file.find_last_of("."), 5, ".test") == 0) {
        return new SimpleParser;
    } else {
        return new AssimpParser;
    }
}
