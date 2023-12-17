#include <lightwave.hpp>

namespace lightwave {
/**
 * Models the BRDF/BSDF (light reflection) for a simple diffuse/Lambertian (uniformly scattering) material.
 */
class Diffuse : public Bsdf {
private:
    ref<Texture> m_albedo;

public:
    explicit Diffuse(const Properties& properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
        const Color albedo = m_albedo->evaluate(uv);
        return {albedo * InvPi * Frame::cosTheta(wi)};
    }

    /**
     * Performs the BSDF reflection "in reverse" by generating a random incidence vector for a given reflectance vector.
     * Due to this being a diffuse material, the reflectance vector is irrelevant. The generation of the
     * incidence vector isn't fully random: the density function skews towards the middle of the shading hemisphere.
     * The equation for the weight/color of this sample equals:
     * @code albedo * InvPi / cosineHemispherePdf(wi) * Frame::cosTheta(wi) @endcode
     * Since the terms cancel out, the weight equals just the albedo.
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
