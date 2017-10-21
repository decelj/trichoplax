#pragma once

#include <vector>
#include <glm/glm.hpp>

class Triangle;
class Material;
class Vertex;

class Mesh
{
private:
    typedef std::vector<Triangle*> PrimitivesArray;

public:
    typedef PrimitivesArray::const_iterator ConstPrimIterator;

    Mesh(unsigned numberOfVerticies);
    ~Mesh();

    void addVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv);
    void addPrimitive(unsigned indexA, unsigned indexB, unsigned indexC, Material* material);

    ConstPrimIterator begin() const { return mPrimitives.begin(); }
    ConstPrimIterator end() const   { return mPrimitives.end(); }

private:
    Vertex*         mVertices;
    PrimitivesArray mPrimitives;
    unsigned        mNumVerts;
    unsigned        mCurrentVertexIdx;
};
