#ifndef __HIT_H__
#define __HIT_H__

#include <glm/glm.hpp>

#include "iprimitive.h"

class Ray;
class Material;

class Hit
{
public:
    explicit Hit(const Ray& r);
    
    // All public for speed
    const glm::vec3 P;
    const glm::vec3 N;
    const glm::vec3 I;

private:
    // Not copyable
    Hit(const Hit&) = delete;
    Hit& operator=(const Hit&) = delete;
};

#endif

