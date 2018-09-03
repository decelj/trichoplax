#include <string>
#include <memory>

#include "parser_factory.h"
#include "simple_parser.h"
#include "fbx_importer.h"

std::unique_ptr<IParser> ParserFactory::create(const std::string &file)
{
    if (file.compare(file.find_last_of("."), 5, ".test") == 0)
    {
        return std::make_unique<SimpleParser>();
    }

    return std::make_unique<FBXImporter>();
}
