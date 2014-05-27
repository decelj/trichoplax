#ifndef __TRANSFORM_STACK_H__
#define __TRANSFORM_STACK_H__

#include <glm/glm.hpp>
#include <stack>

class TransformStack {
public:
    TransformStack();
    
    void push();
    void pop();
    void transform(const glm::mat4& m);
    glm::vec3 transformPoint(const glm::vec3& p) const;
    glm::vec3 transformNormal(const glm::vec3& n) const;
    
    void translate(const float x, const float y, const float z);
    void rotate(const glm::vec3& axis, const float degrees);
    void scale(const float x, const float y, const float z);
    
    glm::mat4& top();
    
private:
    std::stack<glm::mat4> mStack;
};

#endif

