#include <lightwave.hpp>

namespace lightwave {

/// @brief A rectangle in the xy-plane, spanning from [-1,-1,0] to [+1,+1,0].
class Rectangle : public Shape {
private:
    inline Point2 getUVCoordinates() {}

    /**
     * @brief Constructs a surface event for a given position, used by @ref intersect to populate the @ref Intersection
     * and by @ref sampleArea to populate the @ref AreaSample .
     * @param surf The surface event to populate with texture coordinates, shading frame and area pdf
     * @param position The hitpoint (i.e., point in [-1,-1,0] to [+1,+1,0]), found via intersection or area sampling
     */
    inline void populate(SurfaceEvent& surf, const Point& position) const {
        surf.position = position;

        // map the position from [-1,-1,0]..[+1,+1,0] to [0,0]..[1,1] by discarding the z component and rescaling
        surf.uv.x() = (position.x() + 1) * 0.5f;
        surf.uv.y() = (position.y() + 1) * 0.5f;

        // the tangent always points in positive x direction
        surf.frame.tangent = Vector(1, 0, 0);
        // the bitagent always points in positive y direction
        surf.frame.bitangent = Vector(0, 1, 0);
        // and accordingly, the normal always points in the positive z direction
        surf.frame.normal = Vector(0, 0, 1);

        // since we sample the area uniformly, the pdf is given by 1/surfaceArea
        surf.pdf = 1.0f * 0.25f;
    }

public:
    explicit Rectangle(const Properties& properties) {}

    bool intersect(const Ray& ray, Intersection& its, Sampler& rng) const override {
        // if the ray travels in the xy-plane, we report no intersection
        // (we ignore the edge case - pun intended - that the ray might have infinite intersections with the rectangle)
        if (ray.direction.z() == 0) {
            return false;
        }

        // ray.origin.z + t * ray.direction.z = 0
        // <=> t = -ray.origin.z / ray.direction.z
        const float t = -ray.origin.z() / ray.direction.z();

        // note that we never report an intersection closer than Epsilon (to avoid self-intersections)!
        // we also do not update the intersection if a closer intersection already exists (i.e., its.t is lower than our own t)
        if (t < Epsilon || t > its.t) {
            return false;
        }

        // compute the hitpoint
        const Point position = ray(t);
        // we have intersected an infinite plane at z=0; now dismiss anything outside of the [-1,-1,0]..[+1,+1,0] domain.
        if (std::abs(position.x()) > 1 || std::abs(position.y()) > 1) {
            return false;
        }

        // If the primitive has an alpha mask, we need to check whether the coordinate is transparent
        if (its.alphaMask) {
            const Point2 uv = {(position.x() + 1.0f) * 0.5f, (position.y() + 1.0f) * 0.5f};
            if (its.alphaMask->scalar(uv) < rng.next()) {
                return false;
            }
        }

        // we have determined there was an intersection! we are now free to change the intersection object and return true.
        its.t = t;
        populate(its, position); // compute the shading frame, texture coordinates and area pdf (same as sampleArea)

        return true;
    }

    Bounds getBoundingBox() const override {
        return {Point{-1, -1, 0}, Point{+1, +1, 0}};
    }

    Point getCentroid() const override {
        return Point(0);
    }

    AreaSample sampleArea(Sampler& rng) const override {
        // sample a random point in [0,0]..[1,1]
        const Point2 rnd = rng.next2D();
        // stretch the random point to [-1,-1]..[+1,+1] and set z=0
        const Point position = {2.0f * rnd.x() - 1.0f, 2.0f * rnd.y() - 1.0f, 0.0f};

        // compute the shading frame, texture coordinates and area pdf (same as intersection)
        AreaSample sample;
        populate(sample, position);
        return sample;
    }

    std::string toString() const override {
        return "Rectangle[]";
    }
};

}

// this informs lightwave to use our class Rectangle whenever a <shape type="rectangle" /> is found in a scene file
REGISTER_SHAPE(Rectangle, "rectangle")
