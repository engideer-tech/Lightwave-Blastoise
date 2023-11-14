#include <lightwave.hpp>

namespace lightwave {
class NormalsIntegrator : public SamplingIntegrator {
private:
    bool remapNormal;

public:
    explicit NormalsIntegrator(const Properties &properties) : SamplingIntegrator(properties) {
        remapNormal = properties.get<bool>("remap", true);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        return {0.f, 0.f, 0.f};
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
}
REGISTER_INTEGRATOR(NormalsIntegrator, "normals")
