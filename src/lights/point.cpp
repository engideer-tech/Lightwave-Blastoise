#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {
    Color m_power;
    Point m_position;

public:
    PointLight(const Properties &properties) {
        m_power = properties.get<Color>("power");
        m_position = properties.get<Point>("position");
    }
    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {

        Vector direction = (m_position - origin).normalized();
        float distance = (m_position - origin).length();
        Color intensity = m_power/(4.0f*Pi*1.0f*1.0f); //formula from the slides,radius assumed to be 1
        Color intensity_after_falloff = (1.0f*1.0f*intensity)/(distance*distance); //inverse square law
        return{
            .wi = direction,
            .weight = intensity_after_falloff,
            .distance = distance,
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("PointLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
