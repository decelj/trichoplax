#include <iostream>
#include <string>

#include "FreeImage.h"
#include "scene.h"
#include "parser_factory.h"
#include "iparser.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Must pass an input scene file!" << std::endl;
        return -1;
    }
    
    FreeImage_Initialise();
    
    std::string sceneFile(argv[1]);
    ParserFactory factory;
    IParser* parser = factory.create(sceneFile);
    std::string outputImage = parser->parse(sceneFile, Scene::instance());
    delete parser;

    Scene::instance()->render(outputImage);
    Scene::destroy();
    
    FreeImage_DeInitialise();
	
    return 0;
}
