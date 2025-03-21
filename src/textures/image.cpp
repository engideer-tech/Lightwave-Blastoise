#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
private:
    enum class BorderMode : std::uint8_t {
        Clamp,
        Repeat,
    };

    enum class FilterMode : std::uint8_t {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

    /**
     * Clamps or wraps the given coordinates according to the image resolution and the selected BorderMode.
     * @param xy a coordinate in the image pixel space, potentially out of image bounds
     * @returns a coordinate in the image pixel space, guaranteed to be inside of the image boundaries.
     */
    inline Point2i handleBorders(Point2i xy) const {
        const Point2i resolution = m_image->resolution();

        if (m_border == BorderMode::Clamp) {
            xy.x() = std::clamp(xy.x(), 0, resolution.x() - 1);
            xy.y() = std::clamp(xy.y(), 0, resolution.y() - 1);
        } else {
            xy.x() = xy.x() % resolution.x();
            if (xy.x() < 0) xy.x() += resolution.x();
            xy.y() = xy.y() % resolution.y();
            if (xy.y() < 0) xy.y() += resolution.y();
        }

        return xy;
    }

public:
    explicit ImageTexture(const Properties& properties) {
        m_image = properties.has("filename") ? std::make_shared<Image>(properties) : properties.getChild<Image>();
        m_exposure = properties.get<float>("exposure", 1.0f);

        m_border = properties.getEnum<BorderMode>("border", BorderMode::Repeat, {
                {"clamp",  BorderMode::Clamp},
                {"repeat", BorderMode::Repeat},
        });

        m_filter = properties.getEnum<FilterMode>("filter", FilterMode::Bilinear, {
                {"nearest",  FilterMode::Nearest},
                {"bilinear", FilterMode::Bilinear},
        });
    }

    /**
     * Takes in normalized texture plane coordinates (uv in [0, 1)), scales them up by the image resolution, and maps
     * them to the corresponding image pixel coordinates.
     * Values outside that interval are either clamped to the border pixels of the image (clamp mode), or wrapped
     * around the image via modulo (repeat mode).
     * In nearest neighbor mode, we simply use the coordinate of the pixel which uv maps to.
     * In bilinear filtering mode, we perform simple anti-aliasing by sampling the colors of the 4 pixels surrounding
     * the given uv-coordinate and interpolate its color value from them.
     */
    Color evaluate(const Point2& uv) const override {
        Point2 uvScaled = {
                uv.x() * static_cast<float>(m_image->resolution().x()),
                (1.0f - uv.y()) * static_cast<float>(m_image->resolution().y())
        };

        switch (m_filter) {
            case FilterMode::Nearest: {
                const Point2i coords = {
                        static_cast<int>(floorf(uvScaled.x())),
                        static_cast<int>(floorf(uvScaled.y()))
                };
                return m_image->get(handleBorders(coords)) * m_exposure;
            }

            case FilterMode::Bilinear: {
                // Which four pixels to choose is determined by the proximity of the given uv-coordinate to the center
                // points of the pixels (which are located at 0.5, 1.5, etc.). Thus, the weights of the 4 colors equal
                // the distances of the uv coordinate to these values.
                // We can get them either by subtracting 0.5 and using floor(), or using round() and adding 0.5 in the
                // weight computation: either way works.
                uvScaled.x() -= 0.5f;
                uvScaled.y() -= 0.5f;

                const int xMin = static_cast<int>(floorf(uvScaled.x()));
                const int xMax = xMin + 1;
                const int yMin = static_cast<int>(floorf(uvScaled.y()));
                const int yMax = yMin + 1;

                const float xMaxWeight = uvScaled.x() - static_cast<float>(xMin);
                const float xMinWeight = 1.0f - xMaxWeight;
                const float yMaxWeight = uvScaled.y() - static_cast<float>(yMin);
                const float yMinWeight = 1.0f - yMaxWeight;

                const Point2i minCoords = handleBorders({xMin, yMin});
                const Point2i maxCoords = handleBorders({xMax, yMax});

                const Color interpolatedColor = xMinWeight * yMinWeight * m_image->get(minCoords)
                                                + xMinWeight * yMaxWeight * m_image->get({minCoords.x(), maxCoords.y()})
                                                + xMaxWeight * yMinWeight * m_image->get({maxCoords.x(), minCoords.y()})
                                                + xMaxWeight * yMaxWeight * m_image->get(maxCoords);

                return interpolatedColor * m_exposure;
            }
        }
    }

    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure
        );
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
