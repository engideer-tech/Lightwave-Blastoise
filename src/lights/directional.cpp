#include <lightwave.hpp>

namespace lightwave {
/**
 * A light so infinitely far away that it's modeled by a single direction vector. All its rays are parallel.
 * Doesn't suffer from light intensity fall-off.
 */
class DirectionalLight final : public Light {
private:
    /// Direction in which the light is shining
    Vector m_direction;
    /// Strength and color of the light
    Color m_intensity;

public:
    explicit DirectionalLight(const Properties& properties) {
        m_direction = properties.get<Vector>("direction").normalized();
        m_intensity = properties.get<Color>("intensity", Color(1.0f));
    }

    DirectLightSample sampleDirect(const Point& origin, Sampler& rng) const override {
        return {m_direction, m_intensity, Infinity};
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
