#pragma once

#include <glm/glm.hpp>


struct Vertex
{
    Vertex();
    Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& _uv);

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};
