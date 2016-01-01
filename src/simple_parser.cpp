#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>

#include "simple_parser.h"
#include "transform_stack.h"
#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "material.h"
#include "point_light.h"
#include "direct_light.h"
#include "scene.h"


namespace {
bool readValues(std::stringstream &lineStream, const int num, float* values)
{
    for (int i = 0; i < num; i++)
    {
        lineStream >> values[i];
        if (lineStream.fail())
        {
            std::cerr << "Failed to read value " << i+1 << " for line \"" <<
                lineStream.str() << "\", skipping command" << std::endl;
            return false;
        }
    }
    
    return true;
}
} // annoymous namespace


std::string SimpleParser::parse(const std::string& file, Scene& scene)
{
    std::ifstream in;
    in.open(file);
    if (!in.is_open())
        throw std::runtime_error("error opening scene file!");
    
    std::string line, cmd, outputImage;
    TransformStack tStack;
    std::vector<glm::vec3> verticies;
    float values[10]; // Buffer for values
    Material* currMaterial = new Material();
    
    // Defaults
    float constAtten = 1.0f;
    float linearAtten = 0.0f;
    float quadAtten = 0.0f;
    float bias = 0.001f;
    float pointLgtRadius = 0.0f;
    unsigned w = 0, h = 0;
    
    do {
        getline(in, line);
        
        // Ignore empty lines and comments
        if ((line[0] == '#') ||
            (line.find_first_not_of(" \t\r\n") == std::string::npos)) {
            continue;
        }
        
        std::stringstream lineStream(line);
        lineStream >> cmd;

        if (cmd == "size") {
            if (readValues(lineStream, 2, values))
            {
                w = static_cast<unsigned>(values[0]);
                h = static_cast<unsigned>(values[1]);
            }
        } else if (cmd == "output") {
            lineStream >> outputImage;
        } else if (cmd == "maxdepth") {
            if (readValues(lineStream, 1, values))
                scene.setMaxDepth(static_cast<unsigned>(values[0]));
        } else if (cmd == "shdwrays") {
            if (readValues(lineStream, 1, values))
                scene.setShadowRays(static_cast<unsigned>(values[0]));
        } else if (cmd == "camera") {
            if (w == 0 || h == 0)
                throw std::runtime_error("zero image height and/or width!");
            
            if (readValues(lineStream, 10, values)) {
                scene.setCamera(new Camera(values[9], // fov
                                           glm::vec3(values[0], values[1], values[2]), // pos
                                           glm::vec3(values[3], values[4], values[5]), // look at
                                           glm::vec3(values[6], values[7], values[8]), // up
                                            w, h) // width/height
                                  );
            }
        } else if (cmd == "vertex") {
            if (readValues(lineStream, 3, values))
                verticies.push_back(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "maxverts") {
            // Using vectors so I don't care
        } else if (cmd == "pushTransform") {
            tStack.push();
        } else if (cmd == "popTransform") {
            tStack.pop();
        } else if (cmd == "sphere") {
            if (readValues(lineStream, 4, values))
                scene.addPrimitive(new Sphere(glm::vec3(values[0], values[1], values[2]),
                                              values[3],
                                              currMaterial->clone(),
                                              tStack.top())
                                     );
        } else if (cmd == "tri") {
            if (readValues(lineStream, 3, values))
                scene.addPrimitive(new Triangle(tStack.transformPoint(verticies[static_cast<size_t>(values[0])]),
                                                tStack.transformPoint(verticies[static_cast<size_t>(values[1])]),
                                                tStack.transformPoint(verticies[static_cast<size_t>(values[2])]),
                                                currMaterial->clone())
                                     );
        } else if (cmd == "envsphere") {
            std::string envImage;
            lineStream >> envImage;
            scene.setEnvSphereImage(envImage);
        } else if (cmd == "translate") {
            if (readValues(lineStream, 3, values))
                tStack.translate(values[0], values[1], values[2]);
        } else if (cmd == "scale") {
            if (readValues(lineStream, 3, values))
                tStack.scale(values[0], values[1], values[2]);
        } else if (cmd == "rotate") {
            if (readValues(lineStream, 4, values))
                tStack.rotate(glm::vec3(values[0], values[1], values[2]), values[3]);
        } else if (cmd == "ambient") {
            if (readValues(lineStream, 3, values))
            {
                currMaterial->setAmbient(
                    gammaToLinear(glm::vec3(values[0], values[1], values[2])));
            }
        } else if (cmd == "emission") {
            if (readValues(lineStream, 3, values))
            {
                currMaterial->setEmissive(
                    gammaToLinear(glm::vec3(values[0], values[1], values[2])));
            }
        } else if (cmd == "diffuse") {
            if (readValues(lineStream, 3, values))
            {
                currMaterial->setDiffuse(
                    gammaToLinear(glm::vec3(values[0], values[1], values[2])));
            }
        } else if (cmd == "specular") {
            if (readValues(lineStream, 3, values))
            {
                currMaterial->setSpecular(
                    gammaToLinear(glm::vec3(values[0], values[1], values[2])));
            }
        } else if (cmd == "transparency") {
            if (readValues(lineStream, 3, values))
                currMaterial->setTransparency(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "shininess") {
            if (readValues(lineStream, 1, values))
                currMaterial->setShininess(values[0]);
        } else if (cmd == "ior") {
            if (readValues(lineStream, 1, values))
                currMaterial->setIor(values[0]);
        } else if (cmd == "radius") {
            if (readValues(lineStream, 1, values))
                pointLgtRadius = values[0];
        } else if (cmd == "point") {
            if (readValues(lineStream, 6, values))
                scene.addLight(new PointLight(glm::vec3(values[0], values[1], values[2]),
                                              glm::vec3(values[3], values[4], values[5]),
                                              pointLgtRadius,
                                              bias, constAtten, linearAtten, quadAtten)
                                 );
        } else if (cmd == "directional") {
            if (readValues(lineStream, 6, values))
                scene.addLight(new DirectLight(glm::vec3(values[0], values[1], values[2]), // direction
                                               glm::vec3(values[3], values[4], values[5]), // color
                                               bias)
                                 );
        } else if (cmd == "attenuation") {
            if (readValues(lineStream, 3, values)) {
                constAtten = values[0];
                linearAtten = values[1];
                quadAtten = values[2];
            }
        } else if (cmd == "bias") {
            if (readValues(lineStream, 1, values))
                bias = values[0];
        } else if (cmd == "gisamples") {
            if (readValues(lineStream, 1, values))
            {
                scene.setNumGISamples(static_cast<unsigned>(values[0]));
            }
        } else {
            std::cerr << "Unknown command: " << cmd << std::endl;
        }
    } while (in);
    
    delete currMaterial;
    return outputImage;
}
