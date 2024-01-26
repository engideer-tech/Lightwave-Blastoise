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

            if (its.instance->emission() != nullptr) {
                result += its.evaluateEmission() * currentWeight;
            }

            // Don't evaluate NEE on last bounce
            if (depth == m_maxDepth - 1) {
                return result;
            }

            const BsdfSample bsdfSample = its.sampleBsdf(rng);
            // Terminate on invalid sample
            if (bsdfSample.isInvalid()) {
                return result;
            }

            // Next-event estimation (shadow ray towards random non-intersectable light)
            if (m_scene->hasLights()) {
                const LightSample sampledLight = m_scene->sampleLight(rng);
                const DirectLightSample sampledLightPoint = sampledLight.light->sampleDirect(its.position, rng);

                if (!sampledLight.light->canBeIntersected() && !sampledLightPoint.isInvalid()) {
                    const Ray shadowRay = {its.position, sampledLightPoint.wi};

                    if (!m_scene->intersect(shadowRay, sampledLightPoint.distance, rng)) {
                        const BsdfEval bsdfEval = its.evaluateBsdf(sampledLightPoint.wi);
                        result += sampledLightPoint.weight * bsdfEval.value * currentWeight / sampledLight.probability;
                    }
                }
            }

            // Preparation for next bounce
            currentWeight *= bsdfSample.weight;
            currentRay = {its.position, bsdfSample.wi};
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
