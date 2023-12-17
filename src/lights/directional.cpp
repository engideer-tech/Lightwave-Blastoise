#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
    Color m_power;
    Point m_position;

public:
    DirectionalLight(const Properties &properties) {
        m_power = properties.get<Color>("power");
        m_position = properties.get<Point>("position");
    }
    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {

        Vector direction = (m_position - origin).normalized();
        Color intensity = m_power;
        return{
                .wi = direction,
                .weight = intensity
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("DirectionalLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")

