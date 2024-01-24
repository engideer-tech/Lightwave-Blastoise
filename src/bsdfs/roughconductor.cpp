#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
private:
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    explicit RoughConductor(const Properties& properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
        if (Frame::cosTheta(wi) == 0.0f || Frame::cosTheta(wo) == 0.0f) {
            return {Color::black()};
        }

        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const float alpha = std::max(1e-3f, sqr(m_roughness->scalar(uv)));

        const Vector normal = (wi + wo).normalized();
        const Color R = m_reflectance->evaluate(uv);
        const float D = microfacet::evaluateGGX(alpha, normal);
        const float G_wi = microfacet::smithG1(alpha, normal, wi);
        const float G_wo = microfacet::smithG1(alpha, normal, wo);

        const Color weight = (R * D * G_wi * G_wo) /
                             (4.0f * abs(Frame::cosTheta(wo)));

        return {weight};
    }

    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        const float alpha = std::max(1e-3f, sqr(m_roughness->scalar(uv)));

        const Vector normal = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D()).normalized();
        const Vector wi = reflect(wo, normal);
        const Color R = m_reflectance->evaluate(uv);
        const float G_wi = microfacet::smithG1(alpha, normal, wi);

        return {wi, R * G_wi};
    }

    std::string toString() const override {
        return tfm::format(
                "RoughConductor[\n"
                "  reflectance = %s,\n"
                "  roughness = %s\n"
                "]",
                indent(m_reflectance), indent(m_roughness)
        );
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
