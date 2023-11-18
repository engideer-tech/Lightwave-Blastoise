#include <lightwave.hpp>

namespace lightwave {
/**
 * @brief A sphere at the origin of its local coordinate system with the radius 1.
 */
class Sphere : public Shape {
private:
    /**
     * Sets the SurfaceEvent data for an Intersection or AreaSample of this object.
     * @param surf pointer to the SurfaceEvent to be populated
     * @param position intersection or area sample position on the object's surface
     */
    inline static void setSurfaceEventData(SurfaceEvent& surf, const Point& position) {
        surf.position = position;

        // TODO: add UV mapping once it is known how spheres are supposed to be unwrapped for Lightwave

        const Vector normal = Vector(position).normalized();
        surf.frame = Frame(normal);
        surf.frame.normal = normal;

        surf.pdf = 0.0f; // TODO
    }

public:
    explicit Sphere(const Properties& properties) {}

    /**
     * Calculates whether the intersection happened and its location using geometric approach. That is, we span a
     * triangle between the ray origin, sphere center, and middle point of two possible intersection points, as well as
     * between the first possible intersection, sphere center, and middle point. Then compute unknown sides to get
     * intersection distance.
     */
    bool intersect(const Ray& ray, Intersection& its, Sampler& rng) const override {
        const Vector L = Point(0.0f) - ray.origin; // ray origin to sphere center vector
        const float tca = L.dot(ray.direction); // project onto ray to get vector from ray origin to middle point
        if (tca < 0) {
            return false;
        }
        const float dSquared = L.dot(L) - tca * tca; // pythagoras; d = vector from sphere origin to middle point
        if (dSquared > 1) { // if longer than radius then no intersection
            return false;
        }
        const float thc = std::sqrt(1 - dSquared); // pythagoras; thc = vector from intersection to middle point
        float t0 = tca - thc;
        float t1 = tca + thc;

        // We want the smaller but positive intersection distance.
        if (t0 > t1) {
            std::swap(t0, t1);
        }
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
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return "Sphere[]";
    }
};
}
REGISTER_SHAPE(Sphere, "sphere")
