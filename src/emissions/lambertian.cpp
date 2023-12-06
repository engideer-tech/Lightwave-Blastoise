#include <lightwave.hpp>

namespace lightwave {

class Lambertian : public Emission {
private:
    ref<Texture> m_emission;

public:
    explicit Lambertian(const Properties& properties) {
        m_emission = properties.get<Texture>("emission");
    }

    /**
     * todo: add doc on this fn being for area lights
     */
    EmissionEval evaluate(const Point2& uv, const Vector& wo) const override {
        // todo: return black if our vector comes from the negative hemisphere (from the backside)
        return {m_emission->evaluate(uv)};
    }

    std::string toString() const override {
        return tfm::format("Lambertian[\n"
                           "  emission = %s\n"
                           "]",
                           indent(m_emission));
    }
};

} // namespace lightwave

REGISTER_EMISSION(Lambertian, "lambertian")
