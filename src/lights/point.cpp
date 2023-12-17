#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {
private:
    /// Position of the light in world coordinates
    Point m_position;
    /// Intensity and color of the light
    Color m_intensity;

public:
    explicit PointLight(const Properties& properties) {
        m_position = properties.get<Point>("position");
        m_intensity = properties.get<Color>("power", Color(1.0f)) * Inv4Pi;
    }

    /**
     * Computes how strong the lighting from this point light is at the given point (light intensity falls off
     * according to the inverse square law).
     * Additionally computes the vector between the point and this light as well as it's length or distance.
     */
    DirectLightSample sampleDirect(const Point& origin, Sampler& rng) const override {
        const Vector wi = m_position - origin;
        const float distanceSquared = wi.lengthSquared();
        const float distance = std::sqrt(distanceSquared);
        const Color weight = m_intensity / distanceSquared;

        return {wi.normalized(), weight, distance};
    }

    bool canBeIntersected() const override {
        return false;
    }

    std::string toString() const override {
        return tfm::format("PointLight[]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
