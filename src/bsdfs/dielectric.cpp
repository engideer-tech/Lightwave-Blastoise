#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
private:
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;
    Vector m_normal = {0.0f, 0.0f, 1.0f};

public:
    explicit Dielectric(const Properties& properties) {
        m_ior = properties.get<Texture>("ior");
        m_reflectance = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    /**
     * The probability of a light sample picking exactly the direction `wi`
     * that results from reflecting or refracting `wo` is zero,
     * hence we can just ignore that case and always return black.
     */
    BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        float eta;
        Vector normal;
        // coming from outside
        if (wo.z() >= 0.0f) {
            eta = m_ior->scalar(uv);
            normal = {0.0f, 0.0f, 1.0f};
        } else {
            eta = 1 / m_ior->scalar(uv);
            normal = {0.0f, 0.0f, -1.0f};
        }

        const float reflectProbability = fresnelDielectric(wo.z(), eta);
        if (rng.next() <= reflectProbability) {
            // reflect
            const Vector wi = reflect(wo, normal);
            const Color weight = m_reflectance->evaluate(uv);
            return {wi, weight};
        } else {
            // refract
            const Vector wi = refract(wo, normal, eta);
            const Color weight = m_transmittance->evaluate(uv);
            return {wi, weight};
        }
    }

    std::string toString() const override {
        return tfm::format(
                "Dielectric[\n"
                "  ior           = %s,\n"
                "  reflectance   = %s,\n"
                "  transmittance = %s\n"
                "]",
                indent(m_ior),
                indent(m_reflectance),
                indent(m_transmittance)
        );
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
