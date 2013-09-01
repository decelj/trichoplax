#include <fstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "parser.h"
#include "transform_stack.h"
#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "material.h"
#include "point_light.h"
#include "direct_light.h"

using namespace std;

bool readValues(string& line, stringstream &s, const int num, float* values)
{
    for (int i = 0; i < num; i++) {
        s >> values[i];
        if (s.fail()) {
            cerr << "Failed to read value " << i+1 << " for line \"" << line << "\", skipping command" << endl;
            return false;
        }
    }
    
    return true;
}

void readFile(const char* file, std::string &outputImage)
{
    string str, cmd;
    ifstream in;
    in.open(file);
    if (!in.is_open())
        throw(-1);

    TransformStack tStack;
    vector<glm::vec3> verticies;
    float values[10]; // Buffer for values
    Material* currMaterial = new Material();
    
    // Defaults
    float constAtten = 1.0f;
    float linearAtten = 0.0f;
    float quadAtten = 0.0f;
    float bias = 0.001f;
    float pointLgtRadius = 0.0f;
    
    // TODO: Fix me
    int w, h;
    
    getline(in, str);
    while (in) {
        // Ignore empty lines and comments
        if ((str[0] == '#') || (str.find_first_not_of(" \t\r\n") == string::npos)) {
            getline(in, str);
            continue;
        }
        
        stringstream s(str);
        s >> cmd;
        
        if (cmd == "size") {
            if (readValues(str, s, 2, values)) {
                w = values[0]; h = values[1];
                Scene::instance()->createBuffer(w, h);
            }
        } else if (cmd == "output") {
            s >> outputImage;
        } else if (cmd == "maxdepth") {
            if (readValues(str, s, 1, values))
                Scene::instance()->setMaxDepth(values[0]);
        } else if (cmd == "shdwrays") {
            if (readValues(str, s, 1, values))
                Scene::instance()->setShadowRays(values[0]);
        } else if (cmd == "camera") {
            if (readValues(str, s, 10, values)) {
                Scene::instance()->setCamera(
                         new Camera(values[9], // fov
                                    glm::vec3(values[0], values[1], values[2]), // pos
                                    glm::vec3(values[3], values[4], values[5]), // look at
                                    glm::vec3(values[6], values[7], values[8]), // up
                                    w, h) // width/height
                                );
            }
        } else if (cmd == "vertex") {
            if (readValues(str, s, 3, values))
                verticies.push_back(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "maxverts") {
            // Using vectors so I don't care
        } else if (cmd == "pushTransform") {
            tStack.push();
        } else if (cmd == "popTransform") {
            tStack.pop();
        } else if (cmd == "sphere") {
            if (readValues(str, s, 4, values)) 
                Scene::instance()->addPrimitive(new Sphere(
                                            glm::vec3(values[0], values[1], values[2]),
                                            values[3],
                                            currMaterial->clone(),
                                            tStack.top())
                                         );
        } else if (cmd == "tri") {
            if (readValues(str, s, 3, values))
                Scene::instance()->addPrimitive(new Triangle(
                                                    tStack.transformPoint(verticies[values[0]]),
                                                    tStack.transformPoint(verticies[values[1]]),
                                                    tStack.transformPoint(verticies[values[2]]),
                                                    currMaterial->clone())
                                         );
        } else if (cmd == "translate") {
            if (readValues(str, s, 3, values))
                tStack.translate(values[0], values[1], values[2]);
        } else if (cmd == "scale") {
            if (readValues(str, s, 3, values))
                tStack.scale(values[0], values[1], values[2]);
        } else if (cmd == "rotate") {
            if (readValues(str, s, 4, values))
                tStack.rotate(glm::vec3(values[0], values[1], values[2]), values[3]);
        } else if (cmd == "ambient") {
            if (readValues(str, s, 3, values))
                currMaterial->setAmbient(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "emission") {
            if (readValues(str, s, 3, values))
                currMaterial->setEmissive(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "diffuse") {
            if (readValues(str, s, 3, values))
                currMaterial->setDiffuse(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "specular") {
            if (readValues(str, s, 3, values))
                currMaterial->setSpecular(glm::vec3(values[0], values[1], values[2]));
        } else if (cmd == "shininess") {
            if (readValues(str, s, 1, values))
                currMaterial->setShininess(values[0]);
        } else if (cmd == "radius") {
            if (readValues(str, s, 1, values))
                pointLgtRadius = values[0];
        } else if (cmd == "point") {
            if (readValues(str, s, 6, values))
                Scene::instance()->addLight(new PointLight(glm::vec3(values[0], values[1], values[2]),
                                               glm::vec3(values[3], values[4], values[5]),
                                               pointLgtRadius,
                                               bias, constAtten, linearAtten, quadAtten)
                                );
        } else if (cmd == "directional") {
            if (readValues(str, s, 6, values))
                Scene::instance()->addLight(new DirectLight(glm::vec3(values[0], values[1], values[2]), // direction
                                                glm::vec3(values[3], values[4], values[5]), // color
                                                bias)
                                );
        } else if (cmd == "attenuation") {
            if (readValues(str, s, 3, values)) {
                constAtten = values[0];
                linearAtten = values[1];
                quadAtten = values[2];
            }
        } else if (cmd == "bias") {
            if (readValues(str, s, 1, values))
                bias = values[0];
        } else {
            cerr << "Unknown command: " << cmd << endl;
        }
        
        getline(in, str);
    }
    
    delete currMaterial;
}
    