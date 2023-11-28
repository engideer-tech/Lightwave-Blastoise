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
        Vector outgoing_direction = squareToCosineHemisphere(Point2(wo.x(),wo.y())).normalized();
        Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        float pdf = cosineHemispherePdf(wi)/(2*Pi);
        Color bsdf_val = m_albedo->evaluate(uv);
        bsdf_val *= (outgoing_direction.z()*wi.z())/pdf;

        BsdfSample sample;
        sample.wi = wi;
        sample.weight = bsdf_val;
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
