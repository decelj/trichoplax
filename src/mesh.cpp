#include "mesh.h"
#include "triangle.h"
#include "common.h"
#include "sphere.h"
#include "triangle.h"
#include "vertex.h"

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
    for (IPrimitive* prim : mPrimitives)
    {
        delete prim;
    }

    delete [] mVertices;
    mVertices = nullptr;
}

void Mesh::addSphere(Sphere* sphere)
{
    TP_ASSERT(mVertices == nullptr);
    mPrimitives.emplace_back(sphere);
}

void Mesh::addPrimitive(unsigned indexA, unsigned indexB, unsigned indexC, Material* material)
{
    mPrimitives.emplace_back(
        new Triangle(&mVertices[indexA], &mVertices[indexB], &mVertices[indexC], material));
}

void Mesh::addVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv)
{
    TP_ASSERT(mCurrentVertexIdx < mNumVerts);
    mVertices[mCurrentVertexIdx++] = Vertex(position, normal, uv);
}
