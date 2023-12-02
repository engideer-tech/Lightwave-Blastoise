#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    explicit Diffuse(const Properties& properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        // still broken

        Vector outgoing_direction = squareToCosineHemisphere(Point2(wo.x(), wo.y())).normalized();
        Vector fuck = squareToUniformHemisphere({0.0f, 0.0f});
        float pdf_out = abs(cosineHemispherePdf(outgoing_direction)) / (2 * Pi);
        Vector wi = squareToCosineHemisphere(rng.next2D());
        float pdf_sampled = abs(cosineHemispherePdf(wi));
        Color albedo = m_albedo->evaluate(uv);

        // return {wi, albedo * (pdf_out / pdf_sampled)};

        // return {
        //         wi,
        //         albedo * InvPi * cosineHemispherePdf(wi) * std::max(0.0f, Vector(0.0f, 0.0f, 1.0f).dot(wi))
        // };

        return {
                wi.normalized(),
                albedo * InvPi * cosineHemispherePdf(wi) * Frame::cosTheta(wi)
        };

        // Vector wi2 = squareToUniformHemisphere(rng.next2D()).normalized();
        //
        // return {
        //         wi2,
        //         albedo * InvPi * std::max(0.0f, Vector(0.0f, 0.0f, 1.0f).dot(wi2))
        // };
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
