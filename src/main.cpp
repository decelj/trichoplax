#include <iostream>
#include <string>

#include "FreeImage.h"
#include "scene.h"
#include "parser_factory.h"
#include "iparser.h"
#include "timer.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Must provide an input scene file!" << std::endl;
        return -1;
    }
    
    FreeImage_Initialise();
    Scene::create();
    
    std::string sceneFile(argv[1]);
    std::string outputImage;
    
    {
        HighResTimer loadTimer;
        loadTimer.start();
        
        IParser* parser = ParserFactory().create(sceneFile);
        
        try {
            outputImage = parser->parse(sceneFile, Scene::instance());
        } catch (...) {
            delete parser;
            Scene::destroy();
            throw;
        }
        
        delete parser;
        
        std::cout << "Scene load time: "
            << loadTimer.elapsedToString(loadTimer.elapsed()) << std::endl;
    }

    Scene::instance().render(outputImage);
    Scene::destroy();
    
    FreeImage_DeInitialise();
	
    return 0;
}
