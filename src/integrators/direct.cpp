//
// Created by chinm on 26-11-2023.
//
#include <lightwave.hpp>

namespace lightwave {
/**
 * Renders objects by using their surface normal coordinates as RGB color values.
 */
class DirectIntegrator : public SamplingIntegrator {
private:
    bool remap;

public:
    explicit DirectIntegrator(const Properties &properties) : SamplingIntegrator(properties) {
        remap = properties.get<bool>("remap", true);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Ray currentRay = ray;
        Color finalColor(0.0f); // Initialize final color
        int maxBounces = 1;
        for (int bounce = 0; bounce < maxBounces; ++bounce) {
            Intersection intersection = m_scene->intersect(currentRay, rng);

            if (!intersection) {
                // No intersection; return background color
                BackgroundLightEval backgroundLightEval = m_scene->evaluateBackground((currentRay.direction));
                finalColor = finalColor + backgroundLightEval.value;
                break;
            }

            // Sample the BSDF
            BsdfSample bsdfSample = intersection.sampleBsdf(rng);

            // Update the ray direction and weight based on the BSDF sample
            currentRay = Ray(intersection.position, bsdfSample.wi);
            finalColor += bsdfSample.weight; // Accumulate contribution

            // Check if the secondary ray escapes the scene
            if (!m_scene->intersect(currentRay, rng)) {
                BackgroundLightEval backgroundLightEval = m_scene->evaluateBackground((currentRay.direction));
                finalColor = finalColor + backgroundLightEval.value;
                break;
            }
        }

        return finalColor;
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

