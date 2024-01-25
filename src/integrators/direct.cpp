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
            // TODO: ask tutor on why this early return is needed.
            //  If it's not there, in emission.xml you always multiply the light sphere colors by the blue sphere.
            return its1.evaluateEmission();
        }

        // Next-event estimation (shadow ray + lighting data collection)
        // when implementing area lights, you need not just the pdf for selecting that light (which is in degree units)
        // as well as the pdf for selecting that point on the area light (which is in area units). Thus, we will need
        // to convert the area pdf to a degree pdf one. See photo gallery for how to do that.
        if (m_scene->hasLights()) {
            const LightSample sampleLight = m_scene->sampleLight(rng);
            if (!sampleLight.light->canBeIntersected()) {
                const DirectLightSample sampleLightData = sampleLight.light->sampleDirect(its1.position, rng);
                const Ray shadowRay = {its1.position, sampleLightData.wi.normalized()};
                if (!m_scene->intersect(shadowRay, sampleLightData.distance, rng)) {
                    const BsdfEval bsdfEval = its1.evaluateBsdf(sampleLightData.wi.normalized());
                    result += (sampleLightData.weight / sampleLight.probability) * bsdfEval.value;
                }
            }
        }

        const BsdfSample bsdfSample = its1.sampleBsdf(rng);
        // TODO: ask tutor how to properly handle invalid bsdfSamples


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

