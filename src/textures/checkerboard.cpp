#include <lightwave.hpp>

namespace lightwave {

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
        Point2 scaledUV = {uv.x() * m_scale.x(), uv.y() * m_scale.y()};

        // Calculate checkerboard pattern
        // todo: cast before mod, only compute 1 mod
        bool isEven = static_cast<int>(floorf(scaledUV.x())) % 2 == 0;
        bool isEven2 = static_cast<int>(floorf(scaledUV.y())) % 2 == 0;
        if ((isEven && isEven2) || (!isEven && !isEven2)) {
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
