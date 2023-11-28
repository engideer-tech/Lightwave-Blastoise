#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    Diffuse(const Properties &properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        //NOT_IMPLEMENTED
        Intersection its;
        Vector outgoing_direction = squareToCosineHemisphere(Point2(wo.x(),wo.y())).normalized();
        float pdf_out = abs(cosineHemispherePdf(outgoing_direction))/(2*Pi);
        Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        float pdf_sampled = abs(cosineHemispherePdf(wi));
        Color albedo = m_albedo->evaluate(uv);

        BsdfSample sample;
        sample.wi = wi;
        sample.weight = albedo*(pdf_out/pdf_sampled);
        return sample;
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
