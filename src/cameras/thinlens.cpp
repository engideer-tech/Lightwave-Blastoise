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
class Thinlens : public Camera {
private:
    float m_xScalar;
    float m_yScalar;
    float m_lensRadius;
    float m_focalDistance;

public:
    /**
     * Precomputes X and Y scaling factors which transform the normalized image plane coordinates to a
     * vector direction within the 3D local camera coordinate system, where the image plane is at z = 1.
     * This is done by spanning a triangle in the camera's coordinate system, where A = camera, B = center of plane,
     * and C = edge of image plane on the fovAxis. To find C, we can use tan() = BC/AB.
     */
    explicit Thinlens(const Properties& properties) : Camera(properties) {
        const float fov = properties.get<float>("fov", 90.0f);
        const std::string fovAxis = properties.get<std::string>("fovAxis", "x");
        // Default settings mimic pinhole camera
        m_lensRadius = properties.get<float>("lensRadius", 0.0f);
        m_focalDistance = properties.get<float>("focalDistance", 1.0f);

        const float fovEdge = tanf(fov * 0.5f * Deg2Rad);

        if (fovAxis == "x") {
            m_xScalar = fovEdge;
            m_yScalar = fovEdge * (static_cast<float>(m_resolution.y()) / static_cast<float>(m_resolution.x()));
        } else {
            m_xScalar = fovEdge * (static_cast<float>(m_resolution.x()) / static_cast<float>(m_resolution.y()));
            m_yScalar = fovEdge;
        }
    }

    /**
     * Implements a thinlens camera; a theoretical approximation of a single-lens camera system. Instead of a pinhole
     * we have have an aperture the size of our lens. The larger the lens radius, the shallower the depth of field.
     * The focal distance is the distance between our lens and the plane of focus in the scene.
     * For each given image coordinate, we sample a random point on the lens as the ray origin. We know that all
     * objects which lay in the focus plane are in focus, meaning all light rays from point x on that object correspond
     * to the same point x' on the sensor, regardless of their path through the lens. We also know that in the pinhole
     * camera model everything is always in focus. Thus, to find the direction for our thinlens-ray, we compute the
     * intersection of the pinhole-ray for that image point x' with the plane of focus, and shoot our thinlens-ray
     * towards that intersection. If the object is indeed in the plane of focus, we will hit the same point x,
     * otherwise we won't and we'll get something blurry instead.
     */
    CameraSample sample(const Point2& normalized, Sampler& rng) const override {
        // Sample random point on lens
        const Point2 lensSample = m_lensRadius * static_cast<Vector2>(squareToUniformDiskConcentric(rng.next2D()));
        const Point rayOrigin = {lensSample.x(), lensSample.y(), 0.0f};

        // Compute thinlens ray direction
        const Vector pinholeRayDirection = {
                normalized.x() * m_xScalar,
                normalized.y() * m_yScalar,
                1.0f
        };
        const Point intersection = pinholeRayDirection * m_focalDistance;
        const Vector thinlensRayDirection = intersection - rayOrigin;

        // Transform from camera to world coordinates
        return {
                .ray = Ray(m_transform->apply(rayOrigin),
                           m_transform->apply(thinlensRayDirection).normalized()),
                .weight = Color(1.0f)
        };
    }

    std::string toString() const override {
        return tfm::format(
                "Thinlens[\n"
                "  width = %d, height = %d,\n"
                "  xScalar = %d, yScalar = %d,\n"
                "  lensRadius = %d, focalDistance = %d,\n"
                "  transform = %s,\n"
                "]",
                m_resolution.x(), m_resolution.y(),
                m_xScalar, m_yScalar,
                m_lensRadius, m_focalDistance,
                indent(m_transform)
        );
    }
};

} // namespace lightwave

REGISTER_CAMERA(Thinlens, "thinlens")
