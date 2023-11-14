#include <lightwave.hpp>

namespace lightwave {
class Sphere : public Shape {
public:
    explicit Sphere(const Properties &properties) {
    }

    bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override {
        return false;
    }

    Bounds getBoundingBox() const override {
        return {};
    }

    Point getCentroid() const override {
        return {};
    }

    AreaSample sampleArea(Sampler &rng) const override {
        return {};
    }

    std::string toString() const override {
        return "Sphere[]";
    }
};
}
REGISTER_SHAPE(Sphere, "sphere")
