#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {
/**
 * Transforms the position and frame of the SurfaceEvent from object to world coordinates.
 */
void Instance::transformFrame(SurfaceEvent& surf) const {
    if (!m_transform) {
        return;
    }

    surf.position = m_transform->apply(surf.position);
    surf.frame.tangent = m_transform->apply(surf.frame.tangent).normalized();
    surf.frame.bitangent = m_transform->apply(surf.frame.bitangent).normalized();
    if (m_flipNormal) {
        surf.frame.bitangent *= -1.0f;
    }

    surf.frame.normal = surf.frame.tangent.cross(surf.frame.bitangent).normalized();
    surf.frame.bitangent = surf.frame.normal.cross(surf.frame.tangent).normalized();
}

/**
 * Transforms the ray and the last intersection distance its.t down to the local object coordinates, if there is a
 * transform applied to this instance in the scene. Then performs the intersection for this instance's object.
 */
bool Instance::intersect(const Ray& worldRay, Intersection& its, Sampler& rng) const {
    // fast path, if no transform is needed
    if (!m_transform) {
        if (m_shape->intersect(worldRay, its, rng)) {
            its.instance = this;
            return true;
        }
        return false;
    }

    const float previousT = its.t;
    Ray localRay = m_transform->inverse(worldRay);
    const float transformedRayDirLength = localRay.direction.length();
    its.t *= transformedRayDirLength;
    localRay = localRay.normalized();

    if (m_shape->intersect(localRay, its, rng)) {
        its.instance = this;
        transformFrame(its);
        its.t = (its.position - worldRay.origin).length();
        return true;
    }

    its.t = previousT;
    return false;
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(Sampler& rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample);
    return sample;
}

}

REGISTER_CLASS(Instance, "instance", "default")
