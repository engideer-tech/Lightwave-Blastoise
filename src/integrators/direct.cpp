#include <lightwave.hpp>

namespace lightwave {
/**
 * Renders objects by tracing a single path the camera of `maxBounces` length at a time. The path originates at the
 * camera and collects reflectance information at each bounce.
 */
class DirectIntegrator : public SamplingIntegrator {
private:
    static constexpr short maxBounces = 1;

public:
    explicit DirectIntegrator(const Properties& properties) : SamplingIntegrator(properties) {}

    Color Li(const Ray& ray, Sampler& rng) override {
        Ray currentRay = ray;
        Color result = Color::white();

        for (short bounce = 0; bounce <= maxBounces; bounce++) {
            const Intersection its = m_scene->intersect(currentRay, rng);
            if (!its) {
                const BackgroundLightEval bgLight = m_scene->evaluateBackground(currentRay.direction);
                return result * bgLight.value;
            }

            // If the last bounce hits an object which is not a light, the path turns black.
            // If we hit a light, we terminate the path.
            if (bounce == maxBounces || its.instance->emission()) {
                return result * its.evaluateEmission();
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

