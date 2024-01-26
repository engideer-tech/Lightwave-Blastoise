#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {
private:
    ref<Instance> m_instance;

public:
    explicit AreaLight(const Properties& properties) {
        m_instance = properties.getChild<Instance>();
    }

    DirectLightSample sampleDirect(const Point& origin, Sampler& rng) const override {
        const AreaSample sample = m_instance->sampleArea(rng);
        const Vector wi = sample.position - origin;
        const float distanceSquared = wi.lengthSquared();
        const float distance = std::sqrt(distanceSquared);

        if (m_instance->emission() == nullptr) {
            return {wi.normalized(), Color::black(), distance};
        }

        const Color emission = m_instance->emission()->evaluate(sample.uv, sample.frame.toLocal(wi)).value
                               / (distanceSquared * sample.pdf);

        return {wi.normalized(), emission, distance};
    }

    bool canBeIntersected() const override {
        return m_instance->isVisible();
    }

    std::string toString() const override {
        return tfm::format(
                "AreaLight[\n"
                "  instance = %s\n"
                "]",
                indent(m_instance)
        );
    }
};

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")
