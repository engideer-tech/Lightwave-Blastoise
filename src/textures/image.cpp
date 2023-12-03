#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
private:
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

    float imageWidth;
    float imageHeight;

public:
    explicit ImageTexture(const Properties& properties) {
        m_image = properties.has("filename") ? std::make_shared<Image>(properties) : properties.getChild<Image>();
        imageWidth = static_cast<float>(m_image->resolution().x());
        imageHeight = static_cast<float>(m_image->resolution().y());

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
        float xScaled = uv.x() * imageWidth;
        float yScaled = (1.0f - uv.y()) * imageHeight;

        if (m_filter == FilterMode::Nearest) {
            int xCoord = static_cast<int>(floorf(xScaled));
            int yCoord = static_cast<int>(floorf(yScaled));

            if (m_border == BorderMode::Clamp) {
                xCoord = std::clamp(xCoord, 0, m_image->resolution().x() - 1);
                yCoord = std::clamp(yCoord, 0, m_image->resolution().y() - 1);
            } else {
                xCoord = xCoord % m_image->resolution().x();
                yCoord = yCoord % m_image->resolution().y();
                if (xCoord < 0) xCoord += m_image->resolution().x();
                if (yCoord < 0) yCoord += m_image->resolution().y();
            }

            return m_image->get(Point2i(xCoord, yCoord)) * m_exposure;
        } else {
            xScaled -= 0.5f;
            yScaled -= 0.5f;

            int xMax, xMin, yMax, yMin;
            if (m_border == BorderMode::Clamp) {
                xScaled = std::clamp(xScaled, 0.0f, imageWidth - 1.0f);
                yScaled = std::clamp(yScaled, 0.0f, imageHeight - 1.0f);

                xMax = std::clamp(static_cast<int>(ceilf(xScaled)), 0, m_image->resolution().x() - 1);
                xMin = std::clamp(static_cast<int>(floorf(xScaled)), 0, m_image->resolution().x() - 1);
                yMax = std::clamp(static_cast<int>(ceilf(yScaled)), 0, m_image->resolution().y() - 1);
                yMin = std::clamp(static_cast<int>(floorf(yScaled)), 0, m_image->resolution().y() - 1);
            } else {
                xScaled = fmodf(xScaled, imageWidth);
                if (xScaled < 0.0f) xScaled += imageWidth;
                yScaled = fmodf(yScaled, imageWidth);
                if (yScaled < 0.0f) yScaled += imageHeight;

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
