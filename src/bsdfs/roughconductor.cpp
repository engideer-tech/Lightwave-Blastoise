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
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const float alpha = std::max(1e-3f, sqr(m_roughness->scalar(uv)));

        const Vector normal = (wi + wo).normalized();
        const float D = microfacet::evaluateGGX(alpha, normal);
        const float G_wi = microfacet::smithG1(alpha, normal, wi);
        const float G_wo = microfacet::smithG1(alpha, normal, wo);

        return {(m_reflectance->evaluate(uv) * D * G_wi * G_wo) /
                (4.0f * Frame::cosTheta(wi) * Frame::cosTheta(wo))};
    }

    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        const float alpha = std::max(1e-3f, sqr(m_roughness->scalar(uv)));

        const Vector normal = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D()).normalized();
        const float normalPdf = microfacet::pdfGGXVNDF(alpha, normal, wo);
        const float wiPdf = normalPdf * microfacet::detReflection(normal, wo);
        const Vector wi = reflect(wo, normal);
        const Color weight = m_reflectance->evaluate(uv);

        return {
                .wi = wi,
                .weight = weight
        };

        // hints:
        // * do not forget to cancel out as many terms from your equations as possible!
        //   (the resulting sample weight is only a product of two factors)
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
