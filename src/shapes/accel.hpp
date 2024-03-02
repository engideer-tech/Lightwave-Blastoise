#pragma once

#include <lightwave/core.hpp>
#include <lightwave/math.hpp>
#include <lightwave/shape.hpp>

#include <numeric>

namespace lightwave {

/**
 * @brief Parent class for shapes that combine many individual shapes (e.g.,
 * triangle meshes), and hence benefit from building an acceleration structure
 * over their children.
 *
 * To use this class, you will need to implement the following methods:
 * - numberOfPrimitives()           -- report the number of individual children
 * that the shape has
 * - intersect(primitiveIndex, ...) -- intersect a single child (identified by
 * the given index) for the given ray
 * - getBoundingBox(primitiveIndex) -- return the bounding box of a single child
 * (used for building the BVH)
 * - getCentroid(primitiveIndex)    -- return the centroid of a single child
 * (used for building the BVH)
 *
 * @example For a simple example of how to use this class, look at @ref
 * shapes/group.cpp
 * @see Group
 * @see TriangleMesh
 */
class AccelerationStructure : public Shape {
private:
    /// @brief The datatype used to index BVH nodes and the primitive index remapping.
    using NodeIndex = int32_t;
    /// @brief The number of bins to use when computing an optimal SAH split.
    static constexpr int BIN_NUM = 16;

    /// @brief A node in our binary BVH tree.
    struct Node {
        /// @brief The axis aligned bounding box of this node.
        Bounds aabb;
        /**
         * @brief Either the index of the left child node in m_nodes (for
         * internal nodes), or the first primitive in m_primitiveIndices (for
         * leaf nodes).
         * @note For efficiency, we store the BVH nodes so that the right child
         * always directly follows the left child, i.e., the index of the right
         * child is always @code leftFirst + 1 @endcode .
         * @note For efficiency, we store primitives so that children of a leaf
         * node are always contiguous in m_primitiveIndices.
         */
        NodeIndex leftFirst;
        /// @brief The number of primitives in a leaf node, or 0 to indicate
        /// that this node is not a leaf node.
        NodeIndex primitiveCount;

        /// @brief Whether this BVH node is a leaf node.
        bool isLeaf() const { return primitiveCount != 0; }

        /// @brief For internal nodes: The index of the left child node in
        /// m_nodes.
        NodeIndex leftChildIndex() const { return leftFirst; }

        /// @brief For internal nodes: The index of the right child node in
        /// m_nodes.
        NodeIndex rightChildIndex() const { return leftFirst + 1; }

        /// @brief For leaf nodes: The first index in m_primitiveIndices.
        NodeIndex firstPrimitiveIndex() const { return leftFirst; }

        /// @brief For leaf nodes: The last index in m_primitiveIndices (still
        /// included).
        NodeIndex lastPrimitiveIndex() const {
            return leftFirst + primitiveCount - 1;
        }
    };

    /// @brief Represents one SAH bin. That is, a grouping of those primitives
    /// of which the centroid points are within some slice of the parent AABB.
    struct Bin {
        Bounds aabb;
        NodeIndex primitiveCount = 0;
    };

    /// @brief A list of all BVH nodes.
    std::vector<Node> m_nodes;
    /**
     * @brief Mapping from internal @c NodeIndex to @c primitiveIndex as used by
     * all interface methods. For efficient storage, we assume that children of
     * BVH leaf nodes have contiguous indices, which would require re-ordering
     * the primitives. For simplicity, we instead perform this re-ordering on a
     * list of indices (which starts of as @code 0, 1, 2, ..., primitiveCount -
     * 1 @endcode ), which allows us to translate from re-ordered (contiguous)
     * indices to the indices the user of this class expects.
     */
    std::vector<int> m_primitiveIndices;

    /// @brief Returns the root BVH node.
    const Node& rootNode() const {
        // by convention, this is always the first element of m_nodes
        return m_nodes.front();
    }

    /**
     * @brief Intersects a BVH node, recursing into children (for internal
     * nodes), or intersecting all primitives (for leaf nodes).
     */
    bool intersectNode(const Node& node, const Ray& ray, Intersection& its, Sampler& rng) const {
        // update the statistic tracking how many BVH nodes have been tested for
        // intersection
        its.stats.bvhCounter++;

        bool wasIntersected = false;
        if (node.isLeaf()) {
            for (NodeIndex i = 0; i < node.primitiveCount; i++) {
                // update the statistic tracking how many children have been
                // tested for intersection
                its.stats.primCounter++;
                // test the child for intersection
                wasIntersected |= intersect(m_primitiveIndices[node.leftFirst + i], ray, its, rng);
            }
        } else { // internal node
            // test which bounding box is intersected first by the ray.
            // this allows us to traverse the children in the order they are
            // intersected in, which can help prune a lot of unnecessary
            // intersection tests.
            const float leftT = intersectAABB(m_nodes[node.leftChildIndex()].aabb, ray);
            const float rightT = intersectAABB(m_nodes[node.rightChildIndex()].aabb, ray);
            if (leftT < rightT) { // left child is hit first; test left child first, then right child
                if (leftT < its.t)
                    wasIntersected |= intersectNode(m_nodes[node.leftChildIndex()], ray, its, rng);
                if (rightT < its.t)
                    wasIntersected |= intersectNode(m_nodes[node.rightChildIndex()], ray, its, rng);
            } else { // right child is hit first; test right child first, then left child
                if (rightT < its.t)
                    wasIntersected |= intersectNode(m_nodes[node.rightChildIndex()], ray, its, rng);
                if (leftT < its.t)
                    wasIntersected |= intersectNode(m_nodes[node.leftChildIndex()], ray, its, rng);
            }
        }
        return wasIntersected;
    }

    /// @brief Performs a slab test to intersect a bounding box with a ray,
    /// returning Infinity in case the ray misses.
    float intersectAABB(const Bounds& bounds, const Ray& ray) const {
        // intersect all axes at once with the minimum slabs of the bounding box
        // you could save the ray.dir inverse in the ray to avoid 6 divisions,
        // but this only saves us ~1%, so let's not do it.
        const auto t1 = (bounds.min() - ray.origin) / ray.direction;
        // intersect all axes at once with the maximum slabs of the bounding box
        const auto t2 = (bounds.max() - ray.origin) / ray.direction;

        // the elementwiseMin picks the near slab for each axis, of which we
        // then take the maximum
        const float tNear = elementwiseMin(t1, t2).maxComponent();
        // the elementwiseMax picks the far slab for each axis, of which we then
        // take the minimum
        const float tFar = elementwiseMax(t1, t2).minComponent();

        if (tFar < tNear) {
            return Infinity; // the ray does not intersect the bounding box
        }
        if (tFar < Epsilon) {
            return Infinity; // the bounding box lies behind the ray origin
        }

        return tNear; // return the first intersection with the bounding box
        // (may also be negative!)
    }

    /// @brief Computes the axis aligned bounding box for a leaf BVH node
    void computeAABB(Node& node) {
        node.aabb = Bounds::empty();
        for (NodeIndex i = 0; i < node.primitiveCount; i++) {
            const Bounds childAABB = getBoundingBox(m_primitiveIndices[node.leftFirst + i]);
            node.aabb.extend(childAABB);
        }
    }

    /// @brief Computes the surface area of a bounding box.
    float surfaceArea(const Bounds& bounds) const {
        const auto size = bounds.diagonal();
        return 2 * (size.x() * size.y() + size.x() * size.z() + size.y() * size.z());
    }

    /// @see getBoundingPoints
    struct BoundingPoints {
        float minBound, maxBound;
    };

    /// @brief Finds the two outermost centroids of the primitives of the given node, along the given axis.
    BoundingPoints getBoundingPoints(const Node& node, const short axis) const {
        float minBound = Infinity;
        float maxBound = -Infinity;

        for (int i = 0; i < node.primitiveCount; i++) {
            const int primitiveIndex = m_primitiveIndices[node.leftFirst + i];
            const float primitiveCenter = getCentroid(primitiveIndex)[axis];
            if (primitiveCenter < minBound) minBound = primitiveCenter;
            if (primitiveCenter > maxBound) maxBound = primitiveCenter;
        }

        return {minBound, maxBound};
    }

    struct SplitParameters {
        short axis = 0;
        float cost = Infinity;
        float position = 0.0f;
    };

    /**
     * Attempts to find the best split plane utilizing a binned SAH algorithm. For this, we define
     * @code splitPlaneCost = primCountLeft * aabbSurfaceAreaLeft + primCountRight * aabbSurfaceAreaRight @endcode
     * To find the split plane with the lowest cost, we subdivide the parent AABB into N bins along some axis.
     * We then evaluate the SAH cost at each of the N-1 split planes.
     * To avoid looping over all primitives for every split, we group the primitives into the bins based on their
     * centroids and calculate left and right totals for all splits.
     * @param node node to be split up
     * @return best split axis, cost, and position
     * @see https://jacco.ompf2.com/2022/04/21/how-to-build-a-bvh-part-3-quick-builds/
     */
    SplitParameters findBestSplit(const Node& node) const {
        SplitParameters result = {};

        for (short axis = 0; axis < 3; axis++) {
            // Use bounds defined by outermost centroids. This reduces the effective node AABB size.
            const auto [minBound, maxBound] = getBoundingPoints(node, axis);
            if (minBound == maxBound) {
                return {};
            }

            // Populate the bins
            std::array<Bin, BIN_NUM> bins;
            const float scale = BIN_NUM / (maxBound - minBound); // inverse of bin size

            for (int i = 0; i < node.primitiveCount; i++) {
                const int primitiveIndex = m_primitiveIndices[node.leftFirst + i];
                const float primitiveCenter = getCentroid(primitiveIndex)[axis];
                const int binIndex = std::min(BIN_NUM - 1, static_cast<int>((primitiveCenter - minBound) * scale));
                bins[binIndex].primitiveCount++;
                bins[binIndex].aabb.extend(getBoundingBox(primitiveIndex));
            }

            // Sum up the left and right areas and primitive counts for all split positions
            std::array<float, BIN_NUM - 1> leftAreas{}, rightAreas{};
            std::array<int, BIN_NUM - 1> leftCounts{}, rightCounts{};

            Bounds leftBoundTotal, rightBoundTotal;
            int leftCountTotal = 0, rightCountTotal = 0;

            for (int i = 0; i < BIN_NUM - 1; i++) {
                leftCountTotal += bins[i].primitiveCount;
                leftCounts[i] = leftCountTotal;

                leftBoundTotal.extend(bins[i].aabb);
                leftAreas[i] = surfaceArea(leftBoundTotal);

                rightCountTotal += bins[BIN_NUM - 1 - i].primitiveCount;
                rightCounts[BIN_NUM - 2 - i] = rightCountTotal; // -1 to get index & -1 cause array is one smaller => -2

                rightBoundTotal.extend(bins[BIN_NUM - 1 - i].aabb);
                rightAreas[BIN_NUM - 2 - i] = surfaceArea(rightBoundTotal);
            }

            // Calculate SAH cost for all split positions
            const float binSize = (maxBound - minBound) / BIN_NUM;
            for (int i = 0; i < BIN_NUM - 1; i++) {
                const float candidateCost = leftCounts[i] * leftAreas[i] + rightCounts[i] * rightAreas[i];
                if (candidateCost < result.cost) {
                    result.axis = axis;
                    result.cost = candidateCost;
                    result.position = minBound + binSize * (i + 1);
                }
            }
        }

        return result;
    }

    /// @brief Attempts to subdivide a given BVH node.
    void subdivide(Node& parent) {
        // only subdivide if enough children are available
        if (parent.primitiveCount <= 2) {
            return;
        }

        const auto [splitAxis, splitCost, splitPosition] = findBestSplit(parent);

        // abort subdivision if its resulting cost would be worse than unsplitted parent's cost
        const float parentCost = surfaceArea(parent.aabb) * parent.primitiveCount;
        if (splitCost >= parentCost) {
            return;
        }

        // partition algorithm (similar to quicksort)
        // the primitives must be re-ordered so that all children of the left node will have a smaller index than
        // firstRightIndex, and nodes on the right will have an index larger or equal to firstRightIndex
        const NodeIndex firstPrimitive = parent.firstPrimitiveIndex();
        NodeIndex firstRightIndex = firstPrimitive;
        NodeIndex lastLeftIndex = parent.lastPrimitiveIndex();
        while (firstRightIndex <= lastLeftIndex) {
            if (getCentroid(m_primitiveIndices[firstRightIndex])[splitAxis] < splitPosition) {
                firstRightIndex++;
            } else {
                std::swap(m_primitiveIndices[firstRightIndex], m_primitiveIndices[lastLeftIndex--]);
            }
        }

        const NodeIndex leftCount = firstRightIndex - parent.firstPrimitiveIndex();
        const NodeIndex rightCount = parent.primitiveCount - leftCount;
        // if either child gets no primitives, we abort subdividing
        if (leftCount == 0 || rightCount == 0) {
            return;
        }

        // the two children will always be contiguous in our m_nodes list
        const auto leftChildIndex = static_cast<NodeIndex>(m_nodes.size());
        const auto rightChildIndex = leftChildIndex + 1;
        // mark the parent node as internal node
        parent.primitiveCount = 0;
        parent.leftFirst = leftChildIndex;

        // create child nodes
        m_nodes.emplace_back();
        m_nodes[leftChildIndex].leftFirst = firstPrimitive;
        m_nodes[leftChildIndex].primitiveCount = leftCount;

        m_nodes.emplace_back();
        m_nodes[rightChildIndex].leftFirst = firstRightIndex;
        m_nodes[rightChildIndex].primitiveCount = rightCount;

        // first, process the left child node (and all of its children)
        computeAABB(m_nodes[leftChildIndex]);
        subdivide(m_nodes[leftChildIndex]);
        // then, process the right child node (and all of its children)
        computeAABB(m_nodes[rightChildIndex]);
        subdivide(m_nodes[rightChildIndex]);
    }

protected:
    /// @brief Returns the number of children (individual shapes) that are part
    /// of this acceleration structure.
    virtual int numberOfPrimitives() const = 0;

    /// @brief Intersect a single child (identified by the index) with the given
    /// ray.
    virtual bool intersect(int primitiveIndex, const Ray& ray, Intersection& its, Sampler& rng) const = 0;

    /// @brief Returns the axis aligned bounding box of the given child.
    virtual Bounds getBoundingBox(int primitiveIndex) const = 0;

    /// @brief Returns the centroid of the given child.
    virtual Point getCentroid(int primitiveIndex) const = 0;

    /// @brief Builds the acceleration structure.
    void buildAccelerationStructure() {
        Timer buildTimer;

        // fill primitive indices with 0 to primitiveCount - 1
        m_primitiveIndices.resize(numberOfPrimitives());
        std::iota(m_primitiveIndices.begin(), m_primitiveIndices.end(), 0);

        // create root node
        Node& root = m_nodes.emplace_back();
        root.leftFirst = 0;
        root.primitiveCount = numberOfPrimitives();
        computeAABB(root);
        subdivide(root);

        logger(EInfo, "built BVH with %ld nodes for %ld primitives in %.1f ms",
               m_nodes.size(), numberOfPrimitives(),
               buildTimer.getElapsedTime() * 1000);
    }

public:
    bool intersect(const Ray& ray, Intersection& its, Sampler& rng) const override {
        // exit early if no children exist
        if (m_primitiveIndices.empty()) {
            return false;
        }

        // test root bounding box for potential hit
        if (intersectAABB(rootNode().aabb, ray) < its.t) {
            return intersectNode(rootNode(), ray, its, rng);
        }

        return false;
    }

    Bounds getBoundingBox() const override {
        return rootNode().aabb;
    }

    Point getCentroid() const override {
        return rootNode().aabb.center();
    }
};

} // namespace lightwave
