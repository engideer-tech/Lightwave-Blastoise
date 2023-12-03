#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        m_border =
            properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                           {
                                               { "clamp", BorderMode::Clamp },
                                               { "repeat", BorderMode::Repeat },
                                           });

        m_filter = properties.getEnum<FilterMode>(
            "filter", FilterMode::Bilinear,
            {
                { "nearest", FilterMode::Nearest },
                { "bilinear", FilterMode::Bilinear },
            });
    }

    Color evaluate(const Point2 &uv) const override {
        float u = uv.x();
        float v = uv.y();
        switch (m_border) {
            case BorderMode::Clamp:
                u = std::clamp(uv.x(), 0.0f, float(m_image->resolution().x()));
                v = std::clamp(uv.y(), 0.0f, float(m_image->resolution().y()));
                break;
            case BorderMode::Repeat:
                u = std::fmod(u,m_image->resolution().x());
                v = std::fmod(v,m_image->resolution().y());
                if (u < 0) u += 1.0f;
                if (v < 0) v += 1.0f;
                break;
            default:
                break;
        }
        float u_texel = u * float(m_image->resolution().x())-1.0;
        float v_texel = v * float(m_image->resolution().y())-1.0;
        if(m_filter==FilterMode::Nearest){
            int newx = floor(u_texel);
            int newy = floor(v_texel);

            float diffx = u_texel - newx;
            float diffy = v_texel - newy;

            if (diffx > 0.5) newx++;
            if (diffy > 0.5) newy++;

            return m_image->get(Point2i(newx, newy));

//            int nearestU = static_cast<int>(std::round(u_texel));
//            int nearestV = static_cast<int>(std::round(v_texel));
//            //std::cout<<"interpolated color nearest"<<m_image->get(Point2i(nearestU,nearestV))*m_exposure<<std::endl;
//            return m_image->get(Point2i(nearestU,nearestV))*m_exposure;
        }else if(m_filter==FilterMode::Bilinear){
            int u0 = static_cast<int>(std::floor(u_texel));
            int v0 = static_cast<int>(std::floor(v_texel));
            int u1 = std::min(u0 + 1, m_image->resolution().x() - 1);
            int v1 = std::min(v0 + 1, m_image->resolution().y() - 1);

            float s = u_texel - u0;
            float t = v_texel - v0;

            Color texel00 = m_image->get(Point2i(u0,v0));
            Color texel10 = m_image->get(Point2i(u1,v0));
            Color texel01 = m_image->get(Point2i(u0,v1));
            Color texel11 = m_image->get(Point2i(u1,v1));

            Color interpolated_color = (1 - s) * (1 - t) * texel00
                                        +s * (1 - t) * texel10
                                        +(1 - s) * t * texel01
                                        +s * t * texel11;
            //std::cout<<"interpolated color"<<interpolated_color*m_exposure<<std::endl;
            return interpolated_color;

        }
    }

    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
