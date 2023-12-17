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
        if(m_scene->hasLights()){
            LightSample light_sampled = m_scene->sampleLight(rng);
            if(!light_sampled.light->canBeIntersected()){

                Point current_shading_point = its1.position;
                DirectLightSample direct_light_sampled = light_sampled.light->sampleDirect(current_shading_point,rng);
                Vector light_direction = direct_light_sampled.wi;
                BsdfEval bsdf = its1.evaluateBsdf(light_direction);
                Ray Shadow_Ray = {its1.position,light_direction};
                Intersection shadow_intersect = m_scene->intersect(Shadow_Ray,rng);
                bool isVisible = shadow_intersect && shadow_intersect.wo.length()>direct_light_sampled.distance;
                //Color result_val = bsdf.value;
                if(isVisible){
                    result += (bsdf.value*direct_light_sampled.weight)/light_sampled.probability;
                }
                return result;
            }
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

