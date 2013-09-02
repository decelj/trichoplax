#include <iostream>
#include <string>

#include "scene.h"
#include "parser.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Must pass an input scene file!" << std::endl;
        return -1;
    }
    
    std::string outputImage;
    readFile(argv[1], outputImage);
    Scene::instance()->render(outputImage);
    Scene::destroy();
	
    return 0;
}
