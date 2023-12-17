#include <lightwave.hpp>

namespace lightwave {
/**
 * Traces a single-hop path from the camera. Thus, the first intersection collects light/reflectance information,
 * and the second one only light info (given the intersections occur). If a path contains no light (neither from an
 * object nor from the background illumination), that path turns black.
 */
class DirectIntegrator : public SamplingIntegrator {
public:
    explicit DirectIntegrator(const Properties& properties) : SamplingIntegrator(properties) {}

    Color Li(const Ray& ray, Sampler& rng) override {
        Color result = Color::white();

        // First ray
        const Intersection its1 = m_scene->intersect(ray, rng);
        if (!its1) {
            return m_scene->evaluateBackground(ray.direction).value;
        }

        if (its1.instance->emission() != nullptr) {
            // TODO: ask tutor on why this early return is needed.
            //  If it's not there, in emission.xml you always multiply the light sphere colors by the blue sphere.
            return result * its1.evaluateEmission();
        }

        if (m_scene->hasLights()) {
            const LightSample sampleLight = m_scene->sampleLight(rng);
            if (!sampleLight.light->canBeIntersected()) {
                const DirectLightSample lightProps = sampleLight.light->sampleDirect(its1.position, rng);
                const Ray shadowRay = {its1.position, lightProps.wi};
                if (m_scene->intersect(shadowRay, lightProps.distance, rng)) {
                    const BsdfEval bsdfEval = its1.evaluateBsdf(lightProps.wi);
                    result += (lightProps.weight * bsdfEval.value) / sampleLight.probability;
                }
            }
        }

        const BsdfSample bsdfSample = its1.sampleBsdf(rng);
        result *= bsdfSample.weight;


        // Second ray
        const Ray ray2 = {its1.position, bsdfSample.wi};
        const Intersection its2 = m_scene->intersect(ray2, rng);
        if (!its2) {
            const Color bgLight = m_scene->evaluateBackground(ray2.direction).value;
            return result * bgLight;
        }

        return result * its2.evaluateEmission();
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

