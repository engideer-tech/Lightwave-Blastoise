#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        Vector halfway_vector = wi+wo/(wi+wo).normalized();
        const float D = microfacet::evaluateGGX(alpha,halfway_vector.normalized());
        const float G_omega_o = microfacet::smithG1(alpha,halfway_vector.normalized(),wo);
        const float G_omega_i = microfacet::smithG1(alpha,halfway_vector.normalized(),wi);

        Color value = (m_reflectance->evaluate(uv)*D*G_omega_i*G_omega_o)/(4.0f*Frame::cosTheta(wi)*Frame::cosTheta(wo));
        return{
            .value=value
        };

        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        Vector normal = microfacet::sampleGGXVNDF(alpha,wo,rng.next2D());
        float pdf = microfacet::pdfGGXVNDF(alpha,normal.normalized(),wo);
        Vector sampled_normal = normal.normalized();
        Vector wi = reflect(wo,sampled_normal.normalized());
        Color weight = m_reflectance->evaluate(uv);
        return {
            .wi = wi.normalized(),
            .weight = weight
        };


        
        // hints:
        // * do not forget to cancel out as many terms from your equations as possible!
        //   (the resulting sample weight is only a product of two factors)
    }

    std::string toString() const override {
        return tfm::format("RoughConductor[\n"
                           "  reflectance = %s,\n"
                           "  roughness = %s\n"
                           "]",
                           indent(m_reflectance), indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
