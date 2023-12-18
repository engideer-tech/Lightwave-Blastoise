#include <lightwave.hpp>

namespace lightwave {
/**
 * A conductor/specular material is mirror-like: each wi is always reflected to exactly one wo.
 * The conductor may additionally absorb a portion of the light.
 */
class Conductor : public Bsdf {
private:
    ref<Texture> m_reflectance;
    Vector m_normal = {0.0f, 0.0f, 1.0f};

public:
    explicit Conductor(const Properties& properties) {
        m_reflectance = properties.get<Texture>("reflectance");
    }

    /**
     * The probability of a light sample picking exactly the direction `wi`
     * that results from reflecting `wo` is zero,
     * hence we can just ignore that case and always return black.
     */
    BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
        return BsdfEval::invalid();
    }

    /**
     * Since conductor reflections are fully deterministic, rng isn't needed.
     */
    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        const Vector wi = reflect(wo, m_normal);
        const Color reflectance = m_reflectance->evaluate(uv);

        return {wi, reflectance};
    }

    std::string toString() const override {
        return tfm::format(
                "Conductor[\n"
                "  reflectance = %s\n"
                "]",
                indent(m_reflectance)
        );
    }
};

} // namespace lightwave

REGISTER_BSDF(Conductor, "conductor")
