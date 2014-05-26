#include <string>
#include <stdexcept>

#include "parser_factory.h"
#include "simple_parser.h"

IParser* ParserFactory::create(const std::string &file)
{
    if (file.compare(file.find_last_of("."), 5, ".test") == 0) {
        return new SimpleParser;
    } else {
        throw std::logic_error("unknown scene file type!");
    }
}
