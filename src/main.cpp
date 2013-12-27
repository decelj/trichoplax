#include <iostream>
#include <string>

#include "FreeImage.h"
#include "scene.h"
#include "parser.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Must pass an input scene file!" << std::endl;
        return -1;
    }
    
    FreeImage_Initialise();
    
    std::string outputImage;
    readFile(argv[1], outputImage);
    Scene::instance()->render(outputImage);
    Scene::destroy();
    
    FreeImage_DeInitialise();
	
    return 0;
}
