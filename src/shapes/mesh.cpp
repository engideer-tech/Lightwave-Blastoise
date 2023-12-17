#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
 * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
private:
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
     * vertex index (into @c m_vertices ) of the triangle.
     * This list will always contain as many elements as there are triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
     * vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
    bool m_smoothNormals;
    /// @brief Used to avoid self-intersections and other Möller-Trumbore artifacts
    static constexpr float SmallerEpsilon = 1e-8f;
    static constexpr float LargerEpsilon = 1e-4f;

protected:
    int numberOfPrimitives() const override {
        return int(m_triangles.size());
    }

    /**
     * Calculates whether the intersection happened and its location using the Möller-Trumbore algorithm.
     * If the `smooth` property on the mesh is set, the intersection normals are interpolated (Gouraud shading).
     */
    bool intersect(int primitiveIndex, const Ray& ray, Intersection& its, Sampler& rng) const override {
        const Vector3i indices = m_triangles[primitiveIndex];
        const Vertex v0V = m_vertices[indices.x()];
        const Vertex v1V = m_vertices[indices.y()];
        const Vertex v2V = m_vertices[indices.z()];
        const Point v0 = v0V.position;
        const Point v1 = v1V.position;
        const Point v2 = v2V.position;

        const Vector v0v1 = v1 - v0;
        const Vector v0v2 = v2 - v0;
        const Vector pvec = ray.direction.cross(v0v2);
        const float det = v0v1.dot(pvec);
        if (abs(det) < SmallerEpsilon) {
            return false;
        }
        const float invDet = 1 / det;

        const Vector tvec = ray.origin - v0;
        const float u = tvec.dot(pvec) * invDet;
        if (u < 0 || u > 1) {
            return false;
        }

        const Vector qvec = tvec.cross(v0v1);
        const float v = ray.direction.dot(qvec) * invDet;
        if (v < 0 || v > 1 || (u + v) > 1) {
            return false;
        }

        const float t = v0v2.dot(qvec) * invDet;
        if (t < LargerEpsilon || t > its.t) {
            return false;
        }
        its.t = t;
        // TODO: investigate using uv-interpolated position instead of ray multiplication, to ensure point is on triangle
        its.position = ray(t);

        const Vertex interpolatedVertex = Vertex::interpolate({u, v}, v0V, v1V, v2V);

        const Vector normal = m_smoothNormals ? interpolatedVertex.normal.normalized() : v0v1.cross(v0v2).normalized();
        its.frame = Frame(normal);

        its.uv = interpolatedVertex.texcoords;

        its.pdf = 0.0f; // TODO

        return true;
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        const Vector3i indices = m_triangles[primitiveIndex];
        const Point v0 = m_vertices[indices.x()].position;
        const Point v1 = m_vertices[indices.y()].position;
        const Point v2 = m_vertices[indices.z()].position;

        return {elementwiseMin(v0, elementwiseMin(v1, v2)),
                elementwiseMax(v0, elementwiseMax(v1, v2))};
    }

    Point getCentroid(int primitiveIndex) const override {
        const Vector3i indices = m_triangles[primitiveIndex];
        const Point v0 = m_vertices[indices.x()].position;
        const Point v1 = m_vertices[indices.y()].position;
        const Point v2 = m_vertices[indices.z()].position;

        return {(v0.x() + v1.x() + v2.x()) / 3.0f,
                (v0.y() + v1.y() + v2.y()) / 3.0f,
                (v0.z() + v1.z() + v2.z()) / 3.0f};
    }

public:
    explicit TriangleMesh(const Properties& properties) {
        m_originalPath = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath.string(), m_triangles, m_vertices);
        logger(EInfo, "loaded ply with %d triangles, %d vertices",
               m_triangles.size(),
               m_vertices.size()
        );
        buildAccelerationStructure();
    }

    AreaSample sampleArea(Sampler& rng) const override {
        // only implement this if you need triangle mesh area light sampling for your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
                "Mesh[\n"
                "  vertices = %d,\n"
                "  triangles = %d,\n"
                "  filename = \"%s\"\n"
                "]",
                m_vertices.size(),
                m_triangles.size(),
                m_originalPath.generic_string()
        );
    }
};

}

REGISTER_SHAPE(TriangleMesh, "mesh")
