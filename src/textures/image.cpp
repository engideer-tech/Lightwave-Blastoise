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

    /**
     * TODO: write doc
     */
    Color evaluate(const Point2& uv) const override {
        float xScaled = uv.x() * imageWidth;
        float yScaled = (1.0f - uv.y()) * imageHeight;

        switch (m_filter) {
            case FilterMode::Nearest: {
                int xCoord = static_cast<int>(floorf(xScaled));
                int yCoord = static_cast<int>(floorf(yScaled));

                if (m_border == BorderMode::Clamp) {
                    xCoord = std::clamp(xCoord, 0, m_image->resolution().x() - 1);
                    yCoord = std::clamp(yCoord, 0, m_image->resolution().y() - 1);
                } else {
                    xCoord = xCoord % m_image->resolution().x();
                    if (xCoord < 0) xCoord += m_image->resolution().x();
                    yCoord = yCoord % m_image->resolution().y();
                    if (yCoord < 0) yCoord += m_image->resolution().y();
                }

                return m_image->get(Point2i(xCoord, yCoord)) * m_exposure;
            }

            case FilterMode::Bilinear: {
                xScaled -= 0.5f;
                yScaled -= 0.5f;

                // todo: try to convert to int coordinates early and then get 4 neighbors via +/- 0.5 followed by
                //  additional border handling
                if (m_border == BorderMode::Clamp) {
                    xScaled = std::clamp(xScaled, 0.0f, imageWidth - 1.0f);
                    yScaled = std::clamp(yScaled, 0.0f, imageHeight - 1.0f);
                } else {
                    xScaled = fmodf(xScaled, imageWidth);
                    if (xScaled < 0.0f) xScaled += imageWidth;
                    yScaled = fmodf(yScaled, imageHeight);
                    if (yScaled < 0.0f) yScaled += imageHeight;
                }

                const int xMin = static_cast<int>(floorf(xScaled));
                const int yMin = static_cast<int>(floorf(yScaled));
                int xMax = xMin + 1;
                int yMax = yMin + 1;

                if (m_border == BorderMode::Clamp) {
                    xMax = std::clamp(xMax, 0, m_image->resolution().x() - 1);
                    yMax = std::clamp(yMax, 0, m_image->resolution().y() - 1);
                } else {
                    xMax = xMax % m_image->resolution().x();
                    if (xMax < 0) xMax += m_image->resolution().x();
                    yMax = yMax % m_image->resolution().y();
                    if (yMax < 0) yMax += m_image->resolution().y();
                }

                const float xMaxWeight = xScaled - static_cast<float>(xMin);
                const float xMinWeight = 1.0f - xMaxWeight;
                const float yMaxWeight = yScaled - static_cast<float>(yMin);
                const float yMinWeight = 1.0f - yMaxWeight;

                const Color interpolatedColor = xMinWeight * yMinWeight * m_image->get(Point2i(xMin, yMin))
                                                + xMinWeight * yMaxWeight * m_image->get(Point2i(xMin, yMax))
                                                + xMaxWeight * yMinWeight * m_image->get(Point2i(xMax, yMin))
                                                + xMaxWeight * yMaxWeight * m_image->get(Point2i(xMax, yMax));

                return interpolatedColor * m_exposure;
            }
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
