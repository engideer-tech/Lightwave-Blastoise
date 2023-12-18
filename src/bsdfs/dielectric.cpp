#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {
/**
 * A dielectric material is one which both reflects and refracts (meaning the ray is transmitted into the inside of the
 * material). Examples include water, glass, etc. It is also deterministic like a conductor, meaning each wi results in
 * exactly two wo's.
 * The refracted light portion is not 'reflected' symmetrically into the inside of the medium due a change in the
 * speed of light between the two. This skewness is given by the Index of Reflection: the larger the speed difference,
 * the larger the skewness.
 * The relative amount of reflected light is given by the fresnel equation, with the amount of refracted light being
 * equal to 1 - F.
 */
class Dielectric : public Bsdf {
private:
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

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

    /**
     * Since we don't want to trace two rays, we instead sample the reflected/refracted one based on the relative
     * amount of light going in that direction. Since the BSDF is symmetric, it doesn't matter that we're starting out
     * with wi. The @c fresnelDielectric function also takes care of the total internal reflection case by returning
     * a 1 in that case (thus taking up the entire rng spectrum). Finally, if we're coming from inside instead of the
     * outside of the material, we need to flip the ior/eta and the normal.
     */
    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        float eta;
        Vector normal;
        if (wo.z() >= 0.0f) {
            // wo is coming from outside the material
            eta = m_ior->scalar(uv);
            normal = {0.0f, 0.0f, 1.0f};
        } else {
            // wo is coming from inside the material
            eta = 1 / m_ior->scalar(uv);
            normal = {0.0f, 0.0f, -1.0f};
        }

        Vector wi;
        Color weight;
        const float reflectProbability = fresnelDielectric(wo.z(), eta);
        if (rng.next() <= reflectProbability) {
            // reflect
            wi = reflect(wo, normal);
            weight = m_reflectance->evaluate(uv);
        } else {
            // refract
            wi = refract(wo, normal, eta);
            weight = m_transmittance->evaluate(uv) / sqr(eta);
        }
        return {wi, weight};
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
