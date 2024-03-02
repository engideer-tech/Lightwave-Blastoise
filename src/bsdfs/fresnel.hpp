/**
 * @brief Functions for dealing with Fresnel computations.
 * @file microfacet.hpp
 */

#pragma once

#include <lightwave/color.hpp>
#include <lightwave/math.hpp>

namespace lightwave {

inline float schlickWeight(float cosTheta) {
    float m = saturate(1 - cosTheta);
    return (m * m) * (m * m) * m;
}

/**
 * The Schlick approximation of the Fresnel term.
 * @note See "An Inexpensive BRDF Model for Physically-based Rendering" [Schlick
 * 1994].
 */
template <typename T> inline T schlick(T F0, float cosTheta) {
    return F0 + (1 - F0) * schlickWeight(cosTheta);
}

/**
 * Unpolarized Fresnel term for dielectric materials.
 * @param cosThetaI
 * @param eta the relative IOR (n2 / n1)
 * @returns the cosine of the transmitted ray, or 1 in the case of total internal reflection.
 */
inline float fresnelDielectric(float cosThetaI, const float eta) {
    const float invEta = 1 / eta;
    const float cosThetaTSqr = 1 - sqr(invEta) * (1 - sqr(cosThetaI));
    if (cosThetaTSqr <= 0.0f) {
        // Total internal reflection
        return 1;
    }

    cosThetaI = abs(cosThetaI);
    const float cosThetaT = sqrt(cosThetaTSqr);

    const float Rs = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
    const float Rp = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);

    // Average the power of both polarizations
    return 0.5f * (Rs * Rs + Rp * Rp);
}

} // namespace lightwave
