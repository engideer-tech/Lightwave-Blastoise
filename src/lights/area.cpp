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

        Vector wi = sample.position - origin;
        const float distanceSquared = wi.lengthSquared();
        const float distance = std::sqrt(distanceSquared);
        wi = wi.normalized();

        if (m_instance->emission() == nullptr) {
            return {wi, Color::black(), distance};
        }

        const float cosTheta = abs(sample.frame.normal.dot(wi));
        const Color emission = m_instance->emission()->evaluate(sample.uv, sample.frame.toLocal(-wi)).value
                               * cosTheta / (sample.pdf * distanceSquared);

        return {wi, emission, distance};
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
