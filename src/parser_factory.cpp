#include <string>

#include "parser_factory.h"
#include "simple_parser.h"
#include "fbx_importer.h"

IParser* ParserFactory::create(const std::string &file)
{
    if (file.compare(file.find_last_of("."), 5, ".test") == 0)
    {
        return new SimpleParser;
    }

    return new FBXImporter;
}
