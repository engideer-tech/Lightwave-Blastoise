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
    explicit ImageTexture(const Properties& properties) {
        m_image = properties.has("filename") ? std::make_shared<Image>(properties) : properties.getChild<Image>();
        m_exposure = properties.get<float>("exposure", 1);

        m_border = properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                                  {
                                                          {"clamp",  BorderMode::Clamp},
                                                          {"repeat", BorderMode::Repeat},
                                                  });

        m_filter = properties.getEnum<FilterMode>(
                "filter", FilterMode::Bilinear,
                {
                        {"nearest",  FilterMode::Nearest},
                        {"bilinear", FilterMode::Bilinear},
                });
    }

    Color evaluate(const Point2& uv) const override {
        float u = uv.x();
        float v = 1 - uv.y();

        switch (m_border) {
            case BorderMode::Clamp:
                // u = std::clamp(uv.x(), 0.0f, float(m_image->resolution().x()));
                // v = std::clamp(uv.y(), 0.0f, float(m_image->resolution().y()));
                u = clamp(u, 0.0f, 1.0f);
                v = clamp(v, 0.0f, 1.0f);
                break;

            case BorderMode::Repeat:
                // u = std::fmod(u, m_image->resolution().x());
                // v = std::fmod(v, m_image->resolution().y());
                u = std::fmod(u, 1.0f);
                v = std::fmod(v, 1.0f);
                break;
        }

        const float tu = u * static_cast<float>(m_image->resolution().x()) - 1.0f;
        const float tv = v * static_cast<float>(m_image->resolution().y()) - 1.0f;

        if (m_filter == FilterMode::Nearest) {
            int newx = static_cast<int>(floorf(tu));
            int newy = static_cast<int>(floorf(tv));

            const float diffx = tu - static_cast<float>(newx);
            const float diffy = tv - static_cast<float>(newy);

            if (diffx > 0.5) newx++;
            if (diffy > 0.5) newy++;

            return m_image->get(Point2i(newx, newy));

            // int nearestU = static_cast<int>(std::round(u_texel));
            // int nearestV = static_cast<int>(std::round(v_texel));
            // // std::cout<<"interpolated color nearest"<<m_image->get(Point2i(nearestU,nearestV))*m_exposure<<std::endl;
            // return m_image->get(Point2i(nearestU,nearestV))*m_exposure;
        } else if (m_filter == FilterMode::Bilinear) {
            const int u0 = static_cast<int>(floorf(tu));
            const int v0 = static_cast<int>(floorf(tv));
            const int u1 = std::min(u0 + 1, m_image->resolution().x() - 1);
            const int v1 = std::min(v0 + 1, m_image->resolution().y() - 1);

            const float s = tu - static_cast<float>(u0);
            const float t = tv - static_cast<float>(v0);

            const Color texel00 = m_image->get(Point2i(u0, v0));
            const Color texel10 = m_image->get(Point2i(u1, v0));
            const Color texel01 = m_image->get(Point2i(u0, v1));
            const Color texel11 = m_image->get(Point2i(u1, v1));

            const Color interpolated_color = (1 - s) * (1 - t) * texel00
                                             + s * (1 - t) * texel10
                                             + (1 - s) * t * texel01
                                             + s * t * texel11;
            //std::cout<<"interpolated color"<<interpolated_color*m_exposure<<std::endl;
            return interpolated_color;
        }



        // float tu = uv.x();
        // float tv = 1 - uv.y();
        //
        // switch (m_border) {
        //     case BorderMode::Clamp:
        //         tu = clamp(tu, 0.0f, 1.0f);
        //         tv = clamp(tv, 0.0f, 1.0f);
        //         break;
        //
        //     case BorderMode::Repeat:
        //         tu = std::fmod(tu, 1.0f);
        //         tv = std::fmod(tv, 1.0f);
        //         break;
        // }
        //
        // int a, b;
        // switch (m_filter) {
        //     case FilterMode::Nearest:
        //         a = static_cast<int>(tu * static_cast<float>(m_image->resolution().x()));
        //         b = static_cast<int>(tv * static_cast<float>(m_image->resolution().y()));
        //         break;
        //     case FilterMode::Bilinear:
        //         return {1.0f, 0.0f, 0.0f};
        // }
        //
        // Color fuck = (*m_image)(Point2i(a, b));
        // // std::cout << tfm::format("img res %s x %s\nuv %s tu %s tv %s a %s b %s\nColor %s\n",
        // //                          m_image->resolution().x(), m_image->resolution().y(), uv, tu, tv, a, b, fuck);
        //
        // return fuck;
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
