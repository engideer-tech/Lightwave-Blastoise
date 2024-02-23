#include <lightwave.hpp>

namespace lightwave {
/**
 * @brief A sphere at the origin of its local coordinate system with the radius 1.
 */
class Sphere : public Shape {
private:
    /**
     * Checks whether the given ray distance could lead to a valid intersection, and if so, checks the value of the
     * alpha mask at that position. alpha=0 means the ray always passes through and there is no intersection,
     * alpha=1 means there always is one, and values in between randomly allow *some* rays to pass.
     * In case of intersection, the SurfaceEvent data is also set here.
     */
    static bool intersectsAlphaMask(
            const Ray& ray, Intersection& its, const float rayT, Sampler& rng
    ) {
        if (rayT < Epsilon || rayT > its.t) {
            return false;
        }

        const Vector normal = Vector(ray(rayT)).normalized();
        const Point2 uv = {
                atan2f(normal.x(), normal.z()) * Inv2Pi + 0.5f,
                acosf(normal.y()) * InvPi + 0.5f
        };

        if (its.alphaMask->scalar(uv) < rng.next()) {
            return false;
        }

        its.t = rayT;
        its.position = normal; // normalizing ensures the point is on the surface of the sphere
        its.frame = Frame(normal);
        its.uv = uv;

        // Since we sample the area uniformly, the pdf is given by 1/surfaceArea
        its.pdf = Inv4Pi;

        return true;
    }

    /**
     * Sets the SurfaceEvent data for an Intersection or AreaSample of this object.
     * @param surf pointer to the SurfaceEvent to be populated
     * @param position intersection or area sample position on the object's surface
     */
    static void setSurfaceEventData(SurfaceEvent& surf, const Point& position) {
        const Vector normal = Vector(position).normalized();

        surf.position = normal; // normalizing ensures the point is on the surface of the sphere
        surf.frame = Frame(normal);

        surf.uv = {
                atan2f(normal.x(), normal.z()) * Inv2Pi + 0.5f,
                acosf(normal.y()) * InvPi + 0.5f
        };

        // Since we sample the area uniformly, the pdf is given by 1/surfaceArea
        surf.pdf = Inv4Pi;
    }

public:
    explicit Sphere(const Properties& properties) {}

    /**
     * Calculates whether the intersection happened and its location using a geometric approach. That is, we span a
     * triangle between the ray origin, sphere center, and middle point of two possible intersection points, as well as
     * between the first possible intersection, sphere center, and middle point. Then compute unknown sides to get
     * intersection distance.
     */
    bool intersect(const Ray& ray, Intersection& its, Sampler& rng) const override {
        const Vector L = Point(0.0f) - ray.origin; // ray origin to sphere center vector
        const float tca = L.dot(ray.direction); // project onto ray to get vector from ray origin to middle point
        if (tca < 0.0f) {
            return false;
        }
        const float dSquared = L.dot(L) - tca * tca; // pythagoras; d = vector from sphere origin to middle point
        if (dSquared > 1.0f) { // if longer than radius then no intersection
            return false;
        }
        const float thc = std::sqrt(1.0f - dSquared); // pythagoras; thc = vector from intersection to middle point
        float t0 = tca - thc;
        float t1 = tca + thc;

        if (t0 > t1) {
            std::swap(t0, t1);
        }

        // If the primitive has an alpha mask, we need to check both potential intersections against it
        if (its.alphaMask) {
            return intersectsAlphaMask(ray, its, t0, rng)
                   || intersectsAlphaMask(ray, its, t1, rng);
        }

        // We want the smaller but positive intersection distance
        if (t0 < Epsilon) {
            t0 = t1;
        }

        if (t0 < Epsilon || t0 > its.t) {
            return false;
        }

        its.t = t0;
        const Point position = ray(t0);
        setSurfaceEventData(its, position);

        return true;
    }

    Bounds getBoundingBox() const override {
        return {Point(-1.0f), Point(1.0f)};
    }

    Point getCentroid() const override {
        return Point(0.0f);
    }

    AreaSample sampleArea(Sampler& rng) const override {
        const Point position = squareToUniformSphere(rng.next2D());

        AreaSample sample;
        setSurfaceEventData(sample, position);
        return sample;
    }

    std::string toString() const override {
        return "Sphere[]";
    }
};

} // namespace lightwave

REGISTER_SHAPE(Sphere, "sphere")
