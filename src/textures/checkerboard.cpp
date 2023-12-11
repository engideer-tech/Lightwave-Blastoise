#include <lightwave.hpp>

namespace lightwave {
/**
 * Computes a simple checkerboard texture for objects.
 */
class Checkerboard : public Texture {
private:
    Color m_color0;
    Color m_color1;
    Point2 m_scale;

public:
    explicit Checkerboard(const Properties& properties) {
        m_color0 = properties.get<Color>("color0");
        m_color1 = properties.get<Color>("color1");
        m_scale = properties.get<Point2>("scale", Point2(1.0f));
    }

    Color evaluate(const Point2& uv) const override {
        // Apply scale to UV coordinates
        const int scaledU = static_cast<int>(floorf(uv.x() * m_scale.x()));
        const int scaledV = static_cast<int>(floorf(uv.y() * m_scale.y()));

        // Calculate checkerboard pattern
        const bool uEven = scaledU % 2 == 0;
        const bool vEven = scaledV % 2 == 0;
        if (uEven == vEven) {
            return m_color0;
        }
        return m_color1;
    }

    std::string toString() const override {
        return tfm::format("Checkerboard[\n"
                           "  color0 = %s\n"
                           "  color1 = %s\n"
                           "  scale = %s\n"
                           "]",
                           m_color0, m_color1, m_scale
        );
    }
};

} // namespace lightwave

REGISTER_TEXTURE(Checkerboard, "checkerboard")
