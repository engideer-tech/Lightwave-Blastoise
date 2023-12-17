#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
private:
    ///
    Vector m_direction;
    /// Strength and color of the light
    Color m_intensity;

public:
    explicit DirectionalLight(const Properties& properties) {
        m_direction = properties.get<Vector>("direction").normalized();
        m_intensity = properties.get<Color>("intensity", Color(1.0f));
    }

    DirectLightSample sampleDirect(const Point& origin, Sampler& rng) const override {
        NOT_IMPLEMENTED
    }

    bool canBeIntersected() const override {
        return false;
    }

    std::string toString() const override {
        return tfm::format("DirectionalLight[]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
