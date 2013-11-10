/*
 *  point_light.cpp
 *  raytracer
 *
 *  Created by Justin DeCell on 4/10/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <cstdlib>

#include "point_light.h"
#include "multi_sample_ray.h"

namespace {
    static const float gGoldenAngle = M_PI * (3.0f - sqrtf(5.0f));
}

PointLight::PointLight(glm::vec3 pos, glm::vec3 kd, float radius, float bias,
                       float constAtten, float linearAtten, float quadAtten) : 
    mPos(pos),
    mConstAtten(constAtten),
    mLinearAtten(linearAtten),
    mQuadAtten(quadAtten) ,
    mSqrtShadowSamples(1.0f)
{
    mHasAtten = mLinearAtten > 0.0 || mQuadAtten > 0.0 || mConstAtten != 1.f;
    mKd = kd;
    mBias = bias;
    mRadius = radius;
    mShadowRays = 1;
}

glm::vec3 PointLight::getDir(const glm::vec3& p) const
{
    return glm::normalize(mPos - p);
}

glm::vec3 PointLight::getHalf(const glm::vec3& dirToLgt, const glm::vec3& I) const
{
    return glm::normalize(dirToLgt + I);
}

void PointLight::attenuate(const glm::vec3& P, glm::vec3& result) const
{
    if (!mHasAtten) return;
    
    float distance = glm::length(P - mPos);
    result /= mConstAtten + mLinearAtten * distance + mQuadAtten * distance * distance;
}

bool PointLight::generateShadowRay(MultiSampleRay& r) const
{
    if (r.currentSample() <= 0) return false;
    
    glm::vec3 samplePoint;
    randomPointOnDisk(r.origin(), r.currentSample(), samplePoint);
    
    glm::vec3 dirToLgtSample = samplePoint - r.origin();
    r.setDir(glm::normalize(dirToLgtSample));
    r.setMinDistance(glm::length(dirToLgtSample));
    r.bias(mBias);
    
    r.decrementSampleCount();
    
    return true;
}

void PointLight::randomPointOnDisk(const glm::vec3& P, const unsigned int currentSample, glm::vec3& result) const
{
    // Spread points on disk
    // http://blog.marmakoide.org/?p=1
    float r = sqrtf(static_cast<float>(currentSample)) * mRadius / mSqrtShadowSamples;
    float theta = currentSample * gGoldenAngle;
    
    float x = r * cosf(theta);
    float y = r * sinf(theta);
    
    result.x = x + mPos.x;
    result.y = y + mPos.y;
    
    // Plane eqn, slove for z
    // Nx(X-X0) + Ny(Y-Y0) + Nz(Z-Z0) = 0
    // (Nx(X-X0) + Ny(Y-Y0)) / -Nz + Z0 = Z
    glm::vec3 dirToLgt = glm::normalize(P - mPos);
    result.z = ((dirToLgt.x * x + dirToLgt.y * y) / (-1.0f * dirToLgt.z)) + mPos.z;
}

