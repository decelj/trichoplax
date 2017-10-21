#ifndef __HIT_H__
#define __HIT_H__

#include <glm/glm.hpp>

class Ray;

class Hit
{
public:
    explicit Hit(const Ray& r);

    glm::vec3 toWorld(const glm::vec3& v) const;

    glm::vec3 P;
    const glm::vec3& Ng;
    glm::vec3 N;    // Shading normal
    glm::vec3 V;
    glm::vec3 dPdU;
    glm::vec3 dPdV;
    glm::vec2 uv;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    bool hitBackFace;

private:
    // Not copyable
    Hit(const Hit&) = delete;
    Hit& operator=(const Hit&) = delete;
};


inline glm::vec3 Hit::toWorld(const glm::vec3& v) const
{
    return glm::vec3(tangent.x * v.x + bitangent.x * v.y + N.x * v.z,
                     tangent.y * v.x + bitangent.y * v.y + N.y * v.z,
                     tangent.z * v.x + bitangent.z * v.y + N.z * v.z);
}

#endif

