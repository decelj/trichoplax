#pragma once

#include <vector>
#include <glm/glm.hpp>

class IPrimitive;
class Material;
class Sphere;
class Vertex;

class Mesh
{
private:
    typedef std::vector<IPrimitive*> PrimitivesArray;

public:
    typedef PrimitivesArray::const_iterator ConstPrimIterator;

    Mesh(unsigned numberOfVerticies);
    ~Mesh();

    // TODO: Remove this when we move to only triangles
    void addSphere(Sphere* sphere);
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
