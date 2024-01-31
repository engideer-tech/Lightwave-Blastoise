#include "lightwave.hpp"

namespace lightwave {

class ToneMapping : public Postprocess {
public:
    explicit ToneMapping(const Properties& properties) : Postprocess(properties) {}

    void execute() override {
        m_output->initialize(m_input->resolution());

        float max_l = 0.0f;
        for (const auto pixel: m_input->bounds()) {
            if (m_input->get(pixel).luminance() > max_l) {
                max_l = m_input->get(pixel).luminance();
            }
        }

        for (const auto pixel: m_input->bounds()) {
            const float l = m_input->get(pixel).luminance();

            //This is the most basic Reinhard method
            //const float l_new = l / (1.0f + l);

            //This is the extended Reinhard, which works the best at the moment
            //const float numerator = l * (1.0f + l / sqr(max_l));
            //const float l_new = numerator / (1.0f + l);

            //We can use the below method by Drago et al. to choose the degree of contrast we want, by varying the bias.
            //Bias 0.5 is similar to tone mapping with extended Reinhard
            const float bias = 0.1f;
            const float p1 = 1.0f / log10f(1 + max_l);
            const float p2 = log10f(1.0f + l);
            const float p3 = log10f(2.0f + 8.0f * pow(l / max_l, log10f(bias) / log10f(0.5f)));
            const float l_new = p1 * (p2 / p3);

            m_output->get(pixel) = m_input->get(pixel) * (l_new / l);
        }

        m_output->save();
        std::cout << "Tone Mapped Image is generated with the extended Reinhard method" << std::endl;
    }

    std::string toString() const override {
        return tfm::format("ToneMapping[]");
    }
};

} // namespace lightwave

REGISTER_POSTPROCESS(ToneMapping, "tone_mapping")
