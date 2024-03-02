#include <lightwave.hpp>

namespace lightwave {

class BVHPerformance : public SamplingIntegrator {
private:
    float m_scale;

public:
    explicit BVHPerformance(const Properties& properties) : SamplingIntegrator(properties) {
        m_scale = 1.0f / properties.get<float>("unit", 1.0f);
    }

    Color Li(const Ray& ray, Sampler& rng) override {
        const Intersection its = m_scene->intersect(ray, rng);
        return {its.stats.bvhCounter * m_scale,
                its.stats.primCounter * m_scale,
                0.0f};
    }

    std::string toString() const override {
        return tfm::format("BVHPerformance[\n"
                           "  sampler = %s,\n"
                           "  image = %s,\n"
                           "]",
                           indent(m_sampler), indent(m_image));
    }
};

} // namespace lightwave

REGISTER_INTEGRATOR(BVHPerformance, "bvh")
