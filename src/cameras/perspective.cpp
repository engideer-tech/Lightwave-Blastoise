#include <lightwave.hpp>

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
    static constexpr float angle2 = 90.0f * degToPi;
    float xScalar;
    float yScalar;

public:
    /**
     * Precomputes X and Y scaling factors which transform the normalized image plane coordinates to a
     * vector direction within the 3D local camera coordinate system, where the image plane is at z = 1.
     * This is done by spanning a triangle in the camera's coordinate system, where p1 = camera, p2 = center of plane,
     * and p3 = edge of image plane on the fovAxis. p1 and p2 are known, p3 is computed with Pythagoras
     * and the law of sines, given we know all the triangle's angles.
     */
    explicit Perspective(const Properties& properties) : Camera(properties) {
        const float fov = properties.get<float>("fov", 90);
        const std::string fovAxis = properties.get<std::string>("fovAxis", "x");

        const float p1x = 0.0f, p1y = 0.0f;
        const float p2x = 0.0f, p2y = 1.0f;
        const float angle1 = (fov / 2) * degToPi;

        const float u = p2x - p1x, v = p2y - p1y;
        const float a3 = sqrt(u * u + v * v);
        const float angle3 = Pi - (angle1 + angle2);
        const float a2 = a3 * std::sin(angle2) / std::sin(angle3);
        const float RHS1 = p1x * u - p1y * v + a2 * a3 * std::cos(angle1);
        const float RHS2 = p2y * u - p2x * v - a2 * a3 * std::sin(angle1);
        const float p3x = (1 / (a3 * a3)) * (u * RHS1 - v * RHS2);

        const float resolutionRatio = static_cast<float>(m_resolution.x()) / static_cast<float>(m_resolution.y());
        const float y3 = (fovAxis == "x") != (m_resolution.x() < m_resolution.y()) ? // XOR
                         p3x / resolutionRatio : p3x * resolutionRatio;

        if (fovAxis == "x") {
            xScalar = p3x;
            yScalar = y3;
        } else {
            xScalar = y3;
            yScalar = p3x;
        }
    }

    CameraSample sample(const Point2& normalized, Sampler& rng) const override {
        const Vector directionInCameraSystem = Vector(normalized.x() * xScalar,
                                                      normalized.y() * yScalar,
                                                      1.0f);

        return CameraSample{
                .ray = Ray(m_transform->apply(Point(0.0f, 0.0f, 0.0f)),
                           m_transform->apply(directionInCameraSystem).normalized()
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
