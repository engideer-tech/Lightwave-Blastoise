#include <lightwave.hpp>

namespace lightwave {
/**
 * The direct integrator only collects direct lighting. Thus, it is limited to a single bounce (max 2 intersections).
 * We begin with black and add light, weighted by the material reflectance/BSDF.
 * If the scene contains lights, we also fire a shadow ray at the first intersection towards a randomly chosen light
 * in the scene. If the light isn't occuled, we collect it's lighting data at this point ("Next-Event Estimation").
 * This is only done for non-intersectable lights, since those with rigid bodies could instead be hit traditionally.
 */
class DirectIntegrator : public SamplingIntegrator {
public:
    explicit DirectIntegrator(const Properties& properties) : SamplingIntegrator(properties) {}

    Color Li(const Ray& ray, Sampler& rng) override {
        Color result = Color::black();

        // First ray
        const Intersection its1 = m_scene->intersect(ray, rng);
        if (!its1) {
            return m_scene->evaluateBackground(ray.direction).value;
        }

        if (its1.instance->emission() != nullptr) {
            result += its1.evaluateEmission();
        }

        const BsdfSample bsdfSample = its1.sampleBsdf(rng);
        // Terminate on invalid sample
        if (bsdfSample.isInvalid()) {
            return result;
        }

        // Next-event estimation (shadow ray + lighting data collection)
        if (m_scene->hasLights()) {
            const LightSample sampledLight = m_scene->sampleLight(rng);
            const DirectLightSample sampledLightPoint = sampledLight.light->sampleDirect(its1.position, rng);

            if (!sampledLight.light->canBeIntersected() && !sampledLightPoint.isInvalid()) {
                const Ray shadowRay = {its1.position, sampledLightPoint.wi};

                if (!m_scene->intersect(shadowRay, sampledLightPoint.distance, rng)) {
                    const BsdfEval bsdfEval = its1.evaluateBsdf(sampledLightPoint.wi);
                    result += sampledLightPoint.weight * bsdfEval.value / sampledLight.probability;
                }
            }
        }


        // Second ray
        const Ray ray2 = {its1.position, bsdfSample.wi};

        const Intersection its2 = m_scene->intersect(ray2, rng);
        if (!its2) {
            const Color bgLight = m_scene->evaluateBackground(ray2.direction).value;
            return result + bgLight * bsdfSample.weight;
        }

        if (its2.instance->emission() != nullptr) {
            return result + its2.evaluateEmission() * bsdfSample.weight;
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

} // namespace lightwave

REGISTER_INTEGRATOR(DirectIntegrator, "direct")
