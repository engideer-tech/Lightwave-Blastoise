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
    float m_xScalar;
    float m_yScalar;

public:
    /**
     * Precomputes X and Y scaling factors which transform the normalized image plane coordinates to a
     * vector direction within the 3D local camera coordinate system, where the image plane is at z = 1.
     * This is done by spanning a triangle in the camera's coordinate system, where A = camera, B = center of plane,
     * and C = edge of image plane on the fovAxis. To find C, we can use tan() = BC/AB.
     */
    explicit Perspective(const Properties& properties) : Camera(properties) {
        const float fov = properties.get<float>("fov", 90.0f);
        const std::string fovAxis = properties.get<std::string>("fovAxis", "x");

        const float fovEdge = tanf(fov * 0.5f * Deg2Rad);

        if (fovAxis == "x") {
            m_xScalar = fovEdge;
            m_yScalar = fovEdge * (static_cast<float>(m_resolution.y()) / static_cast<float>(m_resolution.x()));
        } else {
            m_xScalar = fovEdge * (static_cast<float>(m_resolution.x()) / static_cast<float>(m_resolution.y()));
            m_yScalar = fovEdge;
        }
    }

    CameraSample sample(const Point2& normalized, Sampler& rng) const override {
        const Vector directionInCameraSystem = {
                normalized.x() * m_xScalar,
                normalized.y() * m_yScalar,
                1.0f
        };

        return {
                .ray = Ray(m_transform->apply(Point(0.0f)),
                           m_transform->apply(directionInCameraSystem).normalized()),
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

} // namespace lightwave

REGISTER_CAMERA(Perspective, "perspective")
