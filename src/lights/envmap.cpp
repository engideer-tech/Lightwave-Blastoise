#include <lightwave.hpp>

namespace lightwave {

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    ref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    ref<Transform> m_transform;

public:
    EnvironmentMap(const Properties &properties) {
        m_texture   = properties.getChild<Texture>();
        m_transform = properties.getOptionalChild<Transform>();
    }

    BackgroundLightEval evaluate(const Vector &direction) const override {
        Vector2 warped = Vector2(0, 0);
        if (m_transform){
            Vector local_direction;
            local_direction = m_transform->inverse(local_direction);
            float mag_local_direction = sqrt(local_direction.x()*local_direction.x()+local_direction.y()*local_direction.y()+local_direction.z()*local_direction.z());
            float theta = acosf(local_direction.y()/mag_local_direction);
            float phi = atan2f(local_direction.z(), local_direction.x());
            //To remap local coordinates to spherical coordinates:
            float u = 1-(phi / (2 * Pi) + 0.5f);
            //u = clamp(u, 0.0f, 1.0f);
            float v = (theta / Pi);
            //v = clamp(v, 0.0f, 1.0f);
            warped = Vector2(u,v);
        }
        
        // hints:
        // * if (m_transform) { transform direction vector from world to local
        // coordinates }
        // * find the corresponding pixel coordinate for the given local
        // direction
        return {
            .value = m_texture->evaluate(warped),
        };
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector direction = squareToUniformSphere(rng.next2D());
        auto E           = evaluate(direction);

        // implement better importance sampling here, if you ever need it
        // (useful for environment maps with bright tiny light sources, like the
        // sun for example)

        return {
            .wi     = direction,
            .weight = E.value / Inv4Pi,
            .distance = Infinity,
        };
    }

    std::string toString() const override {
        return tfm::format("EnvironmentMap[\n"
                           "  texture = %s,\n"
                           "  transform = %s\n"
                           "]",
                           indent(m_texture), indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
