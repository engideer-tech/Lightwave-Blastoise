#include <lightwave.hpp>

namespace lightwave {
/**
 * A diffuse/Lambertian material scatters light randomly and uniformly.
 */
class Diffuse : public Bsdf {
private:
    ref<Texture> m_albedo;

public:
    explicit Diffuse(const Properties& properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    /**
     * Provides the reflection weight info for a given wi and wo. Since this is a diffuse material, the wo doesn't
     * matter (although it shouldn't be outside the shading hemisphere). The only important parameter is the angle of
     * wi: the closer to 90° it is, the less light gets reflected. This is achieved with the cosTheta term.
     */
    BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
        if (wo.z() <= 0.0f || wi.z() <= 0.0f) {
            return BsdfEval::invalid();
        }

        const Color albedo = m_albedo->evaluate(uv);
        return {albedo * InvPi * Frame::cosTheta(wi)};
    }

    /**
     * Performs the BSDF reflection "in reverse" by generating a random incidence vector for a given reflectance vector.
     * Due to this being a diffuse material, the reflectance vector is irrelevant. The generation of the
     * incidence vector isn't fully random: the density function skews towards the middle of the shading hemisphere.
     * The equation for the weight/color of this sample equals:
     * @code albedo * InvPi * Frame::cosTheta(wi) / cosineHemispherePdf(wi) @endcode
     * Since the terms cancel out, the weight equals just the albedo.
     */
    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        // TODO: ask tutor why this check cannot be performed here (it causes image_modes to turn black).
        // if (wo.z() <= 0.0f) {
        //     return BsdfSample::invalid();
        // }

        const Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        const Color albedo = m_albedo->evaluate(uv);

        return {wi, albedo};
    }

    std::string toString() const override {
        return tfm::format(
                "Diffuse[\n"
                "  albedo = %s\n"
                "]",
                indent(m_albedo)
        );
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
