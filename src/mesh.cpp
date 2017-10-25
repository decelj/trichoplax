#include "mesh.h"
#include "triangle.h"
#include "common.h"
#include "sphere.h"
#include "triangle.h"
#include "vertex.h"

#include <glm/glm.hpp>

Mesh::Mesh(unsigned numberOfVerticies)
    : mVertices(nullptr)
    , mPrimitives()
    , mNumVerts(numberOfVerticies)
    , mCurrentVertexIdx(0)
{
    if (mNumVerts > 0)
    {
        mVertices = new Vertex[mNumVerts];
    }
}

Mesh::~Mesh()
{
    for (Triangle* prim : mPrimitives)
    {
        delete prim;
    }

    delete [] mVertices;
    mVertices = nullptr;
}

void Mesh::addPrimitive(unsigned indexA, unsigned indexB, unsigned indexC, Material* material)
{
    if (indexA == indexB || indexB == indexC)
    {
        return;
    }

    Triangle* newTri = new Triangle(&mVertices[indexA], &mVertices[indexB], &mVertices[indexC], material);

    const AABBox bounds = newTri->bounds();
    unsigned flatCount = 0;
    for (unsigned i = 0; i < 3; ++i)
    {
        if (bounds.ll()[i] == bounds.ur()[i]) ++flatCount;
    }

    if (flatCount > 1)
    {
        delete newTri;
        return;
    }
    
    mPrimitives.emplace_back(newTri);
}

void Mesh::addVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv)
{
    TP_ASSERT(mCurrentVertexIdx < mNumVerts);
    mVertices[mCurrentVertexIdx++] = Vertex(position, normal, uv);
}
