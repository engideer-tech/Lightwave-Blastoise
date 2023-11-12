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
        // hints:
        // * precompute any expensive operations here (most importantly trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image

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

        const float resolutionRatio = static_cast<float>(m_resolution.x()) / static_cast<float>(m_resolution.y());
        const float y3 = x3 / resolutionRatio;

        tfToCameraCoordsSys.setColumn(0, Vector(x3, 0, 0));
        tfToCameraCoordsSys.setColumn(1, Vector(0, y3, 0));
        tfToCameraCoordsSys.setColumn(2, Vector(0, 0, 1));

        // std::cout << "x3: " << x3 << " y3: " << y3 << " alp1 " << alp1 << " alp2 " << alp2 << " alp3 " << alp3 << "\n";
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        // hints:
        // * use m_transform to transform the local camera coordinate system into the world coordinate system

        const Vector directionInCameraCoords = (tfToCameraCoordsSys * Vector(normalized.x(), normalized.x(), 0.f));

        return CameraSample{
                .ray = Ray(Vector(0.0f, 0.0f, 0.0f),
                           m_transform->apply(directionInCameraCoords.normalized())
                ),
                .weight = Color(1.0f)
        };

        // return CameraSample{
        //         .ray = Ray(Vector(normalized.x(), normalized.x(), 0.f),
        //                    Vector(0.f, 0.f, 1.f)),
        //         .weight = Color(1.0f)
        // };
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
