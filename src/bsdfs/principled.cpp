#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector& wo, const Vector& wi) const {
        if (wo.z() <= 0.0f || wi.z() <= 0.0f) {
            return BsdfEval::invalid();
        }

        return {color * InvPi * Frame::cosTheta(wi)};
    }

    BsdfSample sample(const Vector& wo, Sampler& rng) const {
        const Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        return {wi, color};
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector& wo, const Vector& wi) const {
        NOT_IMPLEMENTED

        // hints:
        // * copy your roughconductor bsdf evaluate here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }

    BsdfSample sample(const Vector& wo, Sampler& rng) const {
        NOT_IMPLEMENTED

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2& uv, const Vector& wo) const {
        const Color baseColor = m_baseColor->evaluate(uv);
        const float alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const float specular = m_specular->scalar(uv);
        const float metallic = m_metallic->scalar(uv);
        const float F = specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
                .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
                .alpha = alpha,
                .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const float diffuseAlbedo = diffuseLobe.color.mean();
        const float totalAlbedo = diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
                .diffuseSelectionProb = totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
                .diffuse  = diffuseLobe,
                .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties& properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic = properties.get<Texture>("metallic");
        m_specular = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2& uv, const Vector& wo, const Vector& wi) const override {
        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
        const Combination combination = combine(uv, wo);
        const BsdfEval diffuse = combination.diffuse.evaluate(wo, wi);
        const BsdfEval mettalic = combination.metallic.evaluate(wo, wi);

        return {diffuse.value + mettalic.value};
    }

    BsdfSample sample(const Point2& uv, const Vector& wo, Sampler& rng) const override {
        // hint: sample either `combination.diffuse` (probability
        // `combination.diffuseSelectionProb`) or `combination.metallic`
        const Combination combination = combine(uv, wo);

        if (rng.next() <= combination.diffuseSelectionProb) {
            const BsdfSample sample = combination.diffuse.sample(wo, rng);
            return {sample.wi, sample.weight / combination.diffuseSelectionProb};
        } else {
            const BsdfSample sample = combination.metallic.sample(wo, rng);
            return {sample.wi, sample.weight / (1 - combination.diffuseSelectionProb)};
        }
    }

    std::string toString() const override {
        return tfm::format(
                "Principled[\n"
                "  baseColor = %s,\n"
                "  roughness = %s,\n"
                "  metallic  = %s,\n"
                "  specular  = %s,\n"
                "]",
                indent(m_baseColor), indent(m_roughness),
                indent(m_metallic), indent(m_specular)
        );
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
