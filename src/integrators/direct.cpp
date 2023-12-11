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
        bool hitLight = false;

        // First ray
        const Intersection its1 = m_scene->intersect(ray, rng);
        if (!its1) {
            return m_scene->evaluateBackground(ray.direction).value;
        }

        const BsdfSample bsdfSample = its1.sampleBsdf(rng);
        if (its1.instance->emission() != nullptr) {
            hitLight = true;
            result *= its1.evaluateEmission();
            // TODO: ask tutor on why this early return is needed.
            //  If it's not there, in emission.xml you always multiply the light sphere colors by the blue sphere.
            return result;
        } else {
            result *= bsdfSample.weight;
        }


        // Second ray
        const Ray ray2 = {its1.position, bsdfSample.wi};
        const Intersection its2 = m_scene->intersect(ray2, rng);
        if (!its2) {
            const Color bgLight = m_scene->evaluateBackground(ray2.direction).value;
            if (bgLight == Color(0)) {
                return hitLight ? result : Color::black();
            }
            return result * bgLight;
        }

        const Color emission = its2.evaluateEmission();
        if (emission == Color(0)) {
            return hitLight ? result : Color::black();
        }
        return result * emission;
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

