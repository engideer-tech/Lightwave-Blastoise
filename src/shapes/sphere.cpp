#include <lightwave.hpp>

namespace lightwave {
/**
 * @brief A sphere at the origin of its local coordinate system with the radius 1.
 */
class Sphere : public Shape {
private:
    static bool intersectsAlphaMask(
            const Ray& ray, Intersection& its, const float rayT
    ) {
        if (rayT < Epsilon || rayT > its.t) {
            return false;
        }

        const Point position = ray(rayT);
        const Vector normal = Vector(position).normalized();
        const Point2 uv = {
                atan2f(normal.x(), normal.z()) * Inv2Pi + 0.5f,
                acosf(normal.y()) * InvPi + 0.5f
        };

        if (its.alphaMask->scalar(uv) < Epsilon) {
            return false;
        }

        its.t = rayT;
        its.position = position;
        its.frame = Frame(normal);
        its.uv = uv;
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

        surf.uv.x() = atan2f(normal.x(), normal.z()) * Inv2Pi + 0.5f;
        surf.uv.y() = acosf(normal.y()) * InvPi + 0.5f;

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
        if (tca < 0) {
            return false;
        }
        const float dSquared = L.dot(L) - tca * tca; // pythagoras; d = vector from sphere origin to middle point
        if (dSquared > 1) { // if longer than radius then no intersection
            return false;
        }
        const float thc = std::sqrt(1.0f - dSquared); // pythagoras; thc = vector from intersection to middle point
        float t0 = tca - thc;
        float t1 = tca + thc;

        if (t0 > t1) {
            std::swap(t0, t1);
        }

        // If the sphere has an alpha mask, we need to check both potential intersections against it
        if (its.alphaMask) {
            if (intersectsAlphaMask(ray, its, t0)) {
                return true;
            }
            if (intersectsAlphaMask(ray, its, t1)) {
                return true;
            }
            return false;
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
