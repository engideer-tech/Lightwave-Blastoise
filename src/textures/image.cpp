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
        float x = uv.x();
        float y = 1.0f - uv.y();

        switch (m_border) {
            case BorderMode::Clamp:
                x = clamp(x, 0.0f, 1.0f);
                y = clamp(y, 0.0f, 1.0f);
                break;

            case BorderMode::Repeat:
                x = std::fmod(x, 1.0f);
                y = std::fmod(y, 1.0f);
                if (x < 0.0f) x += 1.0f;
                if (y < 0.0f) y += 1.0f;
                break;
        }

        float xScaled = x * static_cast<float>(m_image->resolution().x());
        float yScaled = y * static_cast<float>(m_image->resolution().y());

        if (m_filter == FilterMode::Nearest) {
            const int xCoord = std::min(static_cast<int>(floorf(xScaled)), m_image->resolution().x() - 1);
            const int yCoord = std::min(static_cast<int>(floorf(yScaled)), m_image->resolution().y() - 1);

            return m_image->get(Point2i(xCoord, yCoord)) * m_exposure;
        } else {
            xScaled -= 0.5f;
            yScaled -= 0.5f;

            int xMax, xMin, yMax, yMin;
            if (m_border == BorderMode::Clamp) {
                xScaled = std::clamp(xScaled, 0.0f, static_cast<float>(m_image->resolution().x() - 1));
                yScaled = std::clamp(yScaled, 0.0f, static_cast<float>(m_image->resolution().y() - 1));

                xMax = std::clamp(static_cast<int>(ceilf(xScaled)), 0, m_image->resolution().x() - 1);
                xMin = std::clamp(static_cast<int>(floorf(xScaled)), 0, m_image->resolution().x() - 1);
                yMax = std::clamp(static_cast<int>(ceilf(yScaled)), 0, m_image->resolution().y() - 1);
                yMin = std::clamp(static_cast<int>(floorf(yScaled)), 0, m_image->resolution().y() - 1);
            } else {
                if (xScaled < 0.0f) xScaled = static_cast<float>(m_image->resolution().x()) - 1.5f;
                if (yScaled < 0.0f) yScaled = static_cast<float>(m_image->resolution().y()) - 1.5f;

                xMax = static_cast<int>(std::roundf(xScaled)) % m_image->resolution().x();
                xMin = static_cast<int>(std::roundf(xScaled)) - 1;
                if (xMin < 0) xMin = m_image->resolution().x() - 1;
                yMax = static_cast<int>(std::roundf(yScaled)) % m_image->resolution().y();
                yMin = static_cast<int>(std::roundf(yScaled)) - 1;
                if (yMin < 0) yMin = m_image->resolution().y() - 1;
            }

            const Color texelA = m_image->get(Point2i(xMin, yMin));
            const Color texelB = m_image->get(Point2i(xMax, yMin));
            const Color texelC = m_image->get(Point2i(xMin, yMax));
            const Color texelD = m_image->get(Point2i(xMax, yMax));

            const float xMaxWeight = xScaled - floorf(xScaled);
            const float xMinWeight = 1.0f - xMaxWeight;
            const float yMaxWeight = yScaled - floorf(yScaled);
            const float yMinWeight = 1.0f - yMaxWeight;

            const Color interpolatedColor = xMinWeight * yMinWeight * texelA
                                            + xMaxWeight * yMinWeight * texelB
                                            + xMinWeight * yMaxWeight * texelC
                                            + xMaxWeight * yMaxWeight * texelD;

            if (std::isnan(interpolatedColor.r()) || interpolatedColor.r() > 1.0f || interpolatedColor.r() < 0.0f
                || std::isnan(interpolatedColor.g()) || interpolatedColor.g() > 1.0f || interpolatedColor.g() < 0.0f
                || std::isnan(interpolatedColor.b()) || interpolatedColor.b() > 1.0f || interpolatedColor.b() < 0.0f) {
                std::cout << tfm::format("%s x %s y %s min %s max %s xMinWeight %s yMinWeight %s\n",
                                         interpolatedColor, xScaled, yScaled, Point2i(xMin, yMin), Point2i(xMax, yMax),
                                         xMaxWeight, yMaxWeight
                );
            }

            return interpolatedColor * m_exposure;
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
