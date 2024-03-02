#include <lightwave.hpp>

namespace lightwave {
/**
 * Renders objects by using their surface normal coordinates as RGB color values.
 */
class NormalsIntegrator : public SamplingIntegrator {
private:
    bool remap;

public:
    explicit NormalsIntegrator(const Properties& properties) : SamplingIntegrator(properties) {
        remap = properties.get<bool>("remap", true);
    }

    Color Li(const Ray& ray, Sampler& rng) override {
        const Intersection intersection = m_scene->intersect(ray, rng);
        const Vector normal = intersection ? intersection.frame.normal : Vector(0.0f);
        return remap ? Color((normal + Vector(1.0f)) * 0.5f) : Color(normal);
    }

    std::string toString() const override {
        return tfm::format(
                "NormalsIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image)
        );
    }
};

} // namespace lightwave

REGISTER_INTEGRATOR(NormalsIntegrator, "normals")
