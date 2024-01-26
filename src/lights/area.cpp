#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {
private:
    ref<Shape> m_shape;

public:
    explicit AreaLight(const Properties& properties) {
        m_shape = properties.getChild<Shape>();
    }

    DirectLightSample sampleDirect(const Point& origin, Sampler& rng) const override {
        const AreaSample sample = m_shape->sampleArea(rng);
        const Vector wi = sample.position - origin;
        return {wi.normalized(), Color::black(), wi.length()};
    }

    bool canBeIntersected() const override {
        return true;
    }

    std::string toString() const override {
        return tfm::format(
                "AreaLight[\n"
                "  shape = %s\n"
                "]",
                indent(m_shape)
        );
    }
};

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")
