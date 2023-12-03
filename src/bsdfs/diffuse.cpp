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

    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        const Color albedo = m_albedo->evaluate(uv);

        const Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        const float wiPdf = cosineHemispherePdf(wi);

        const Color weight = wiPdf == 0.0f
                             ? Color(0.0f)
                             : albedo * InvPi / cosineHemispherePdf(wi) * Frame::cosTheta(wi);

        return {wi, weight};
    }

    std::string toString() const override {
        return tfm::format("Diffuse[\n"
                           "  albedo = %s\n"
                           "]",
                           indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
