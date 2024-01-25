#include <lightwave.hpp>

namespace lightwave {

class PathTracerIntegrator : public SamplingIntegrator {
private:
    int m_maxDepth;

public:
    explicit PathTracerIntegrator(const Properties& properties) : SamplingIntegrator(properties) {
        m_maxDepth = properties.get<int>("depth", 2);
    }

    Color Li(const Ray& ray, Sampler& rng) override {
        Color result = Color::black();
        Ray currentRay = ray;
        Color currentWeight = Color::white();

        for (int depth = 0; depth < m_maxDepth; depth++) {
            const Intersection its = m_scene->intersect(currentRay, rng);
            if (!its) {
                return result + m_scene->evaluateBackground(currentRay.direction).value * currentWeight;
            }

            // Next-event estimation (shadow ray + lighting data collection)
            // when implementing area lights, you need not just the pdf for selecting that light (which is in degree units)
            // as well as the pdf for selecting that point on the area light (which is in area units). Thus, we will need
            // to convert the area pdf to a degree pdf one. See photo gallery for how to do that.
            if (m_scene->hasLights()) {
                const LightSample sampleLight = m_scene->sampleLight(rng);
                if (!sampleLight.light->canBeIntersected()) {
                    const DirectLightSample sampleLightData = sampleLight.light->sampleDirect(its.position, rng);
                    const Ray shadowRay = {its.position, sampleLightData.wi.normalized()};
                    if (!m_scene->intersect(shadowRay, sampleLightData.distance, rng)) {
                        const BsdfEval bsdfEval = its.evaluateBsdf(sampleLightData.wi.normalized());
                        result += (sampleLightData.weight / sampleLight.probability) * bsdfEval.value;
                    }
                }
            }

            if (its.instance->emission() != nullptr) {
                result += its.evaluateEmission() * currentWeight;
            }

            const BsdfSample bsdfSample = its.sampleBsdf(rng);
            currentWeight = bsdfSample.weight;
            currentRay = {its.position, bsdfSample.wi.normalized()};
        }

        return result;
    }

    std::string toString() const override {
        return tfm::format(
                "PathTracerIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image)
        );
    }
};

} // namespace lightwave

REGISTER_INTEGRATOR(PathTracerIntegrator, "pathtracer")
