#include <lightwave.hpp>

namespace lightwave {
/**
 * Models an emissive texture on a diffuse (Lambertian) material.
 */
class Lambertian : public Emission {
private:
    ref<Texture> m_emission;

public:
    explicit Lambertian(const Properties& properties) {
        m_emission = properties.get<Texture>("emission");
    }

    /**
     * Evaluates the emission at the given texture coordinate uv.
     * If the ray hits the backside of the object, the emission equals zero.
     */
    EmissionEval evaluate(const Point2& uv, const Vector& wo) const override {
        if (wo.z() <= 0.0f) {
            return {Color(0.0f)};
        }

        return {m_emission->evaluate(uv)};
    }

    std::string toString() const override {
        return tfm::format("Lambertian[\n"
                           "  emission = %s\n"
                           "]",
                           indent(m_emission)
        );
    }
};

} // namespace lightwave

REGISTER_EMISSION(Lambertian, "lambertian")
