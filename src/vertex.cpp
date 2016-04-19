#include "vertex.h"

Vertex::Vertex()
    : position(0.f)
    , normal(0.f, 1.f, 0.f)
    , uv(0.f)
{
}

Vertex::Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& _uv)
    : position(pos)
    , normal(norm)
    , uv(_uv)
{
}
