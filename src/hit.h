#ifndef __HIT_H__
#define __HIT_H__

#include <glm/glm.hpp>

class Ray;

class Hit
{
public:
    explicit Hit(const Ray& r);

    glm::vec3 P;
    glm::vec3 Ng;   // Interpolated geometric normal
    glm::vec3 N;    // Shading normal (will be flipped Ng if backface hit)
    glm::vec3 V;
    glm::vec3 I;
    bool hitBackFace;

private:
    // Not copyable
    Hit(const Hit&) = delete;
    Hit& operator=(const Hit&) = delete;
};

#endif

