#include <lightwave.hpp>

namespace lightwave {
/**
 * A light-emitting skybox (actually skysphere) around the scene. The walls of the sphere are infinitely far away from
 * the scene, which allows to abstract the scene into a single point located at its center.
 */
class EnvironmentMap final : public BackgroundLight {
private:
    /// @brief The texture to use as background
    ref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    ref<Transform> m_transform;

public:
    explicit EnvironmentMap(const Properties& properties) {
        m_texture = properties.getChild<Texture>();
        m_transform = properties.getOptionalChild<Transform>();
    }

    /**
     * Receives a vector pointing away from the scene, towards the skysphere. If the envmap is transformed (rotated),
     * we first apply that transform to the given vector. The vector is then mapped to spherical coordinates, which
     * can be mapped to texture coordinates.
     */
    BackgroundLightEval evaluate(const Vector& direction) const override {
        const Vector localDirection = m_transform ? m_transform->inverse(direction) : direction;

        const float theta = acosf(localDirection.y());
        const float phi = atan2f(-localDirection.z(), localDirection.x());

        const float u = phi * Inv2Pi + 0.5f;
        const float v = theta * InvPi;

        return {m_texture->evaluate({u, v})};
    }

    DirectLightSample sampleDirect(const Point& origin, Sampler& rng) const override {
        const Vector direction = squareToUniformSphere(rng.next2D()).normalized();
        const Color eval = evaluate(direction).value;

        // implement better importance sampling here, if you ever need it
        // (useful for environment maps with bright tiny light sources, like the
        // sun for example)

        return {
                .wi = direction,
                .weight = eval * Inv4Pi,
                .distance = Infinity,
        };
    }

    std::string toString() const override {
        return tfm::format("EnvironmentMap[\n"
                           "  texture = %s,\n"
                           "  transform = %s\n"
                           "]",
                           indent(m_texture), indent(m_transform)
        );
    }
};

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
