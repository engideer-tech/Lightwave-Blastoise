#include "lightwave.hpp"

namespace lightwave {

class ToneMapping : public Postprocess {

public:
    explicit ToneMapping(const Properties &properties): Postprocess(properties) {
        // m_input = properties.has("filename") ? std::make_shared<Image>(properties) : properties.getChild<Image>();

    }

    void execute() override {

        float max_l = 0.0f;
        m_output->initialize(m_input->resolution());
        for(auto pixel:m_input->bounds()){
            if(m_input->get(pixel).luminance()>max_l){
                max_l = m_input->get(pixel).luminance();
            }
        }


        for(auto pixel:m_input->bounds()){
            float l = m_input->get(pixel).luminance();

            //This is the most basic Reinhard method
            // float l_new = l/(1.0f+l);

            //This is the extended Reinhard, which works the best at the moment
            //float numerator = l*(1.0f+(l/(max_l*max_l)));
            //float l_new = numerator / (1.0f + l);

            //We can use the below method by Drago et. al to choose the degree of contrast we want, by varying the bias.
            //Bias 0.5 is similar to tone mapping with extended reinhard
            float bias = 0.1f;
            float p1 = 1.0f/(log10f(1+max_l));
            float p2 = log10f(1.0f+l);
            float p3 = log10f(2.0f+(8.0f*pow((l/max_l),(log10f(bias)/ log10f(0.5)))));
            float l_new = p1*(p2/p3);

            m_output->get(pixel).r() = m_input->get(pixel).r() * (l_new/l);
            m_output->get(pixel).g() = m_input->get(pixel).g() * (l_new/l);
            m_output->get(pixel).b() = m_input->get(pixel).b() * (l_new/l);
        }
        m_output->saveAt("../scenes/Desk1.exr");
        std::cout<<"Tone Mapped Image is generated with the extended Reinhard method"<<std::endl;
    }

    std::string toString() const override {
        return tfm::format("ToneMapping[]");
    }
};

}
REGISTER_POSTPROCESS(ToneMapping,"tone_mapping");
