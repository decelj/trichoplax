#include <glm/glm.hpp>

#include "brdf.h"

BRDF::BRDF()
    : mKd(0.8f)
    , mKa(0.f)
    , mKe(0.f)
    , mKt(0.f)
    , mKr(0.f)
    , mIOR(1.f)
    , mRoughness(1.f)
{
}

BRDF::BRDF(const glm::vec3& Kd, const glm::vec3& Ka,
      const glm::vec3& Ke, const glm::vec3& Kt, float Kr, float ior,
      float roughness)
    : mKd(Kd)
    , mKa(Ka)
    , mKe(Ke)
    , mKt(Kt)
    , mKr(Kr)
    , mIOR(ior)
    , mRoughness(roughness * roughness) // Disney roughness remappig
{
}
