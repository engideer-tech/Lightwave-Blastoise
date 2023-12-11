#include <lightwave.hpp>

namespace lightwave {
/**
 * Performs a simple BRDF/BSDF computation for a simple diffuse (Lambertian) material. Works "in reverse" by generating
 * a random incidence vector for a given reflectance vector. Due to this being a diffuse material, the reflectance
 * vector is not needed. The generation of the incidence vector isn't fully random: the density function skews towards
 * the middle of the shading hemisphere.
 */
class Diffuse : public Bsdf {
private:
    ref<Texture> m_albedo;

public:
    explicit Diffuse(const Properties& properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        NOT_IMPLEMENTED
    }

    /**
     * Computed a BSDF sample for a diffuse material. The full equation for this is:
     * @code albedo * InvPi / cosineHemispherePdf(wi) * Frame::cosTheta(wi) @endcode,
     * but the terms cancel out, leaving us with just albedo as the weight.
     */
    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        const Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        const Color albedo = m_albedo->evaluate(uv);

        return {wi, albedo};
    }

    std::string toString() const override {
        return tfm::format("Diffuse[\n"
                           "  albedo = %s\n"
                           "]",
                           indent(m_albedo)
        );
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
