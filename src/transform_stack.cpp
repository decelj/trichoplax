#include "transform_stack.h"
#include "common.h"

TransformStack::TransformStack()
{
    // Start with identity
    mStack.push(glm::mat4(1.0));
}

void TransformStack::push()
{
    mStack.push(top());
}

void TransformStack::pop()
{
    mStack.pop();
}

glm::mat4& TransformStack::top()
{
    return mStack.top();
}

void TransformStack::transform(const glm::mat4& m)
{
    glm::mat4& t = top();
    t = t * m;
}

glm::vec3 TransformStack::transformNormal(const glm::vec3& n) const
{
    glm::mat4 tmp = glm::transpose(glm::inverse(mStack.top()));
    return glm::normalize(glm::vec3(tmp * glm::vec4(n, 0.f)));
}

glm::vec3 TransformStack::transformPoint(const glm::vec3& p) const
{
    return glm::vec3(mStack.top() * glm::vec4(p, 1.0));
}

void TransformStack::scale(const float x, const float y, const float z)
{
    glm::mat4 m(1.0f);
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
    transform(m);
}

void TransformStack::translate(const float x, const float y, const float z)
{
    glm::mat4 m(1.0f);
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    transform(m);
}

void TransformStack::rotate(const glm::vec3& axis, const float degrees)
{
    float radians = DEGREES_TO_RADIANS(degrees);
    float cosTheta = cosf(radians);
    float sinTheta = sinf(radians);
    
    glm::vec3 normAxis = glm::normalize(axis);
    
    glm::mat3 aaT(1.0f);
    aaT[0][0] = normAxis.x*normAxis.x*(1.f-cosTheta);
    aaT[1][1] = normAxis.y*normAxis.y*(1.f-cosTheta);
    aaT[2][2] = normAxis.z*normAxis.z*(1.f-cosTheta);
    aaT[0][1] = aaT[1][0] = normAxis.x*normAxis.y*(1.f-cosTheta);
    aaT[0][2] = aaT[2][0] = normAxis.x*normAxis.z*(1.f-cosTheta);
    aaT[1][2] = aaT[2][1] = normAxis.y*normAxis.z*(1.f-cosTheta);
    
    glm::mat3 aStar(0.0f);
    aStar[1][0] = normAxis.z*-1.f*sinTheta;
    aStar[2][0] = normAxis.y*sinTheta;
    aStar[0][1] = normAxis.z*sinTheta;
    aStar[2][1] = normAxis.x*-1.f*sinTheta;
    aStar[0][2] = normAxis.y*-1.f*sinTheta;
    aStar[1][2] = normAxis.x*sinTheta;
    
    transform(glm::mat4(glm::mat3(cosTheta) + aaT + aStar));
}


