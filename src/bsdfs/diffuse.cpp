#include <lightwave.hpp>

namespace lightwave {

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
