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

            // Next-event estimation (shadow ray towards random non-intersectable light)
            if (m_scene->hasLights()) {
                const LightSample sampledLight = m_scene->sampleLight(rng);

                if (!sampledLight.light->canBeIntersected()) {
                    const DirectLightSample sampledLightPoint = sampledLight.light->sampleDirect(its.position, rng);
                    const Vector sampledLightWi = sampledLightPoint.wi.normalized();
                    const Ray shadowRay = {its.position, sampledLightWi};

                    if (!m_scene->intersect(shadowRay, sampledLightPoint.distance, rng)) {
                        const BsdfEval bsdfEval = its.evaluateBsdf(sampledLightWi);
                        result += sampledLightPoint.weight * bsdfEval.value * currentWeight / sampledLight.probability;
                    }
                }
            }

            if (its.instance->emission() != nullptr) {
                result += its.evaluateEmission() * currentWeight;
            }

            const BsdfSample bsdfSample = its.sampleBsdf(rng);
            currentWeight *= bsdfSample.weight;
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
