#ifndef __BRDF_H__
#define __BRDF_H__

#include <glm/glm.hpp>

class BRDF
{
public:
    BRDF();
    BRDF(const glm::vec3& Kd, const glm::vec3& Ka,
          const glm::vec3& Ke, const glm::vec3& Kt,
          float Kr, float ior, float roughness);

    BRDF(const BRDF& other) = default;
    BRDF& operator=(const BRDF& rhs) = default;

    inline const glm::vec3& Kd() const { return mKd; }
    inline const glm::vec3& Ka() const { return mKa; }
    inline const glm::vec3& Ke() const { return mKe; }
    inline const glm::vec3& Kt() const { return mKt; }
    inline float Kr() const { return mKr; }
    inline float IOR() const { return mIOR; }
    inline float roughness() const { return mRoughness; }

    inline void setKd(const glm::vec3& Kd) { mKd = Kd; }
    inline void setKa(const glm::vec3& Ka) { mKa = Ka; }
    inline void setKe(const glm::vec3& Ke) { mKe = Ke; }
    inline void setKt(const glm::vec3& Kt) { mKt = Kt; }
    inline void setKr(float Kr) { mKr = Kr; }
    inline void setIOR(float ior) { mIOR = ior; }
    inline void setRoughness(float roughness)
    { mRoughness = roughness * roughness; } // Disney roughness remapping

private:
    glm::vec3 mKd;
    glm::vec3 mKa;
    glm::vec3 mKe;
    glm::vec3 mKt;
    float mKr;
    float mIOR;
    float mRoughness;
};

#endif

