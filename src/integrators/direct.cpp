#include <lightwave.hpp>

namespace lightwave {

class DirectIntegrator : public SamplingIntegrator {
private:
    static constexpr short maxBounces = 1;

public:
    explicit DirectIntegrator(const Properties& properties) : SamplingIntegrator(properties) {}

    Color Li(const Ray& ray, Sampler& rng) override {
        Ray currentRay = ray;
        Color result(1.0f);

        for (short bounce = 0; bounce <= maxBounces; bounce++) {
            const Intersection its = m_scene->intersect(currentRay, rng);

            if (!its) {
                const BackgroundLightEval bgLight = m_scene->evaluateBackground(currentRay.direction);
                return result * bgLight.value;
            }

            if (bounce == maxBounces) {
                return result * 0.0f;
            }

            const BsdfSample bsdfSample = its.sampleBsdf(rng);
            if (bsdfSample.isInvalid()) {
                return result;
            }

            currentRay = Ray(its.position, bsdfSample.wi);
            result *= bsdfSample.weight;
        }

        return result;
    }

    std::string toString() const override {
        return tfm::format(
                "DirectIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image)
        );
    }
};
}
REGISTER_INTEGRATOR(DirectIntegrator, "direct")

