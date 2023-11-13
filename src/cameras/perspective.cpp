#include <lightwave.hpp>
#include <iostream>

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 * 
 * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
 * Pixels on the left side of the image (@code normalized.x < 0 @endcode) are directed in negative x
 * direction (@code ray.direction.x < 0 @endcode), and pixels at the bottom of the image
 * (@code normalized.y < 0 @endcode) are directed in negative y direction (@code ray.direction.y < 0 @endcode).
 */
class Perspective : public Camera {
private:
    static constexpr float degToPi = Pi / 180.0f;
    static constexpr float alp2 = 90.0f * degToPi;
    Matrix3x3 tfToCameraCoordsSys{};

public:
    Perspective(const Properties &properties) : Camera(properties) {
        const float x1 = 0.0f, y1 = 0.0f;
        const float x2 = 0.0f, y2 = 1.0f;
        const float fov = properties.get<float>("fov");
        const float alp1 = (fov / 2) * degToPi;

        const float u = x2 - x1, v = y2 - y1;
        const float a3 = sqrt(u * u + v * v);
        const float alp3 = Pi - (alp1 + alp2);
        const float a2 = a3 * std::sin(alp2) / std::sin(alp3);
        const float RHS1 = x1 * u - y1 * v + a2 * a3 * std::cos(alp1);
        const float RHS2 = y2 * u - x2 * v - a2 * a3 * std::sin(alp1);
        const float x3 = (1 / (a3 * a3)) * (u * RHS1 - v * RHS2);

        // TODO: calculate differently based on axis of fov
        const float resolutionRatio = static_cast<float>(m_resolution.x()) / static_cast<float>(m_resolution.y());
        const float y3 = x3 / resolutionRatio;

        std::cout << "x3: " << x3 << " y3: " << y3 << "\n";

        tfToCameraCoordsSys.setColumn(0, Vector(x3, 0.0f, 0.0f));
        tfToCameraCoordsSys.setColumn(1, Vector(0.0f, y3, 0.0f));
        tfToCameraCoordsSys.setColumn(2, Vector(0.0f, 0.0f, 1.0f));

        // const Point2 point{-0.435041, 0.216175};
        // const Vector directionInCameraCoords = tfToCameraCoordsSys * Vector(point.x(), point.y(), 0.0f);
        // std::cout << "matrix " << tfToCameraCoordsSys << "\n";
        // std::cout << "point " << point << " direction " << directionInCameraCoords << "\n";
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        const Vector directionInCameraCoords = tfToCameraCoordsSys * Vector(normalized.x(), normalized.y(), 1.0f);

        // TODO: take camera translation into account
        return CameraSample{
                .ray = Ray(Vector(0.0f, 0.0f, 0.0f),
                           m_transform->apply(directionInCameraCoords.normalized())
                ),
                .weight = Color(1.0f)
        };
    }

    std::string toString() const override {
        return tfm::format(
                "Perspective[\n"
                "  width = %d,\n"
                "  height = %d,\n"
                "  transform = %s,\n"
                "]",
                m_resolution.x(),
                m_resolution.y(),
                indent(m_transform)
        );
    }
};

}

REGISTER_CAMERA(Perspective, "perspective")
