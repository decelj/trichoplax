#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>
#include <list>
#include <cstdint>

#include "aligned_allocator.h"
#include "aabbox.h"
#include "common.h"
#include "stats.h"
#include "mailboxer.h"


class Triangle;
class Ray;


class KdTree
{
private:
    using PrimitiveVector = std::vector<const Triangle*, AlignedAllocator<const Triangle*> >;

    class Node
    {
    private:
        enum Flags : uint32_t
        {
            XAXIS = 0x0,
            YAXIS = 0x1,
            ZAXIS = 0x2,
            LEAF = 0x3,
            MASK = 0x3
        };

    public:
        using ConstPrimIterator = const Triangle* const*;
        using PrimIterator = const Triangle**;

        explicit Node();
        ~Node();

        // Don't copy
        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        // But we can move
        Node(Node&& rhs);

        void split(uint32_t leftChild, uint32_t rightChild, float position, uint32_t plane);
        void initLeafNode(uint32_t numPrimitives);

        bool intersect(const Ray& ray) const;

        uint32_t splitAxis() const { return (mFlags & MASK); }
        float splitPlane() const { return mPlanePosition; }
        uint32_t upperChildIdx() const { return mUpperChild >> 2; }
        uint32_t lowerChildIdx() const { return mLowerChild; }

        bool isLeaf() const { return (mFlags & MASK) == LEAF; }
        uint32_t primitiveCount() const { return mPrimCount >> 2; }

        ConstPrimIterator beginPrimitives() const { return mPrimitives; }
        ConstPrimIterator endPrimitives() const { return mPrimitives + primitiveCount(); }

        PrimIterator beginPrimitives() { return mPrimitives; }
        PrimIterator endPrimitives() { return mPrimitives + primitiveCount(); }

    private:
        union // 4 bytes
        {
            uint32_t    mUpperChild;
            uint32_t    mPrimCount;
            Flags       mFlags;
        };

        union // 8 bytes
        {
            struct
            {
                float    mPlanePosition;
                uint32_t mLowerChild;
            };

            struct
            {
                const Triangle** mPrimitives;
            };
        };
    };
    using NodeBuffer = std::vector<Node, AlignedAllocator<Node> >;
    
    enum SHAPlaneEventType
    {
        END = 0,
        PLANAR,
        START
    };
    
    struct SHAPlaneEvent
    {
        SHAPlaneEvent(const Triangle* _prim, float _plane, unsigned _axis,
                      SHAPlaneEventType _type);
        
        bool operator<(const SHAPlaneEvent& rhs) const;
        
        const Triangle* primitive;
        float plane;
        unsigned axis;
        SHAPlaneEventType type;
    };
    using SHAPlaneEventList = std::list<SHAPlaneEvent>;
    
    struct SHASplitPlane
    {
        enum Side
        {
            LEFT = 0,
            RIGHT
        };

        SHASplitPlane();

        float plane;
        float cost;
        unsigned aaAxis;
        Side side;
        unsigned numPrimitivesRight;
        unsigned numPrimitivesLeft;
    };

    struct TraversalState
    {
        const Node* node;
        float minT;
        float maxT;
    };
    
public:
    using TraversalBuffer = std::vector<TraversalState, AlignedAllocator<TraversalState> >;

    explicit KdTree();
    ~KdTree();
    
    void build();

    template <bool visibilityTest>
    bool trace(Ray& ray, TraversalBuffer& traversalStack, Mailboxer& mailboxes, Stats& threadStats) const;
    
    void addPrimitive(const Triangle* p);
    size_t numberOfPrimitives() const { return mPrimVector.size(); }
    TraversalBuffer allocateTraversalBuffer() const;
    
private:
    uint32_t allocNode(uint32_t* nextNodeIdx);

    void build(uint32_t nodeIdx, const AABBox& bounds, SHAPlaneEventList& events, unsigned numPrimitives, unsigned depth, uint32_t* nextNodeIdx);
    void split(SHAPlaneEventList& outLeftEvents, SHAPlaneEventList& outRightEvents,
               const AABBox& leftBounds, const AABBox& rightBounds,
               const SHASplitPlane& plane, SHAPlaneEventList& events);
    SHASplitPlane findSplitPlane(const AABBox& voxel, const SHAPlaneEventList& events,
                                 unsigned totalNumPrimitives) const;
    void SHACost(float* lowestCostOut, SHASplitPlane::Side* outSide,
                 float plane, unsigned aaAxis, const AABBox& voxel,
                 unsigned numLeftPrims, unsigned numRightPrims, unsigned numPlanarPrims) const;

    void generateEventsForPrimitive(const Triangle* primitive, const AABBox& voxel,
                                    SHAPlaneEventList& events) const;

    AABBox                  mBounds;
    NodeBuffer              mNodes;
    unsigned                mMaxDepth;
    unsigned                mMinDepth;
    size_t                  mLeafNodes;
    size_t                  mTotalNodes;
    uint32_t                mMaxPrimsPerNode;
    PrimitiveVector         mPrimVector;
};


inline void KdTree::addPrimitive(const Triangle* p)
{
    mPrimVector.emplace_back(p);
}

inline uint32_t KdTree::allocNode(uint32_t* nextNodeIdx)
{
    uint32_t result = (*nextNodeIdx)++;

    if (result >= mNodes.size())
    {
        mNodes.resize(mNodes.size() << 1);
    }

    ++mTotalNodes;
    return result;
}

inline void KdTree::Node::split(uint32_t leftChild, uint32_t rightChild, float position, uint32_t axis)
{
    TP_ASSERT(leftChild != rightChild);
    TP_ASSERT(isLeaf());
    TP_ASSERT(upperChildIdx() == 0);
    TP_ASSERT(mPrimitives == nullptr);

    mUpperChild = (rightChild << 2) | (axis & MASK);
    mLowerChild = leftChild;
    mPlanePosition = position;
}

inline void KdTree::Node::initLeafNode(uint32_t numPrimitives)
{
    TP_ASSERT(isLeaf());
    TP_ASSERT(upperChildIdx() == 0);
    TP_ASSERT(mPrimitives == nullptr);

    mPrimCount |= numPrimitives << 2;

    if (numPrimitives)
    {
        mPrimitives = new const Triangle*[numPrimitives];
    }
}

inline bool KdTree::SHAPlaneEvent::operator<(const SHAPlaneEvent& rhs) const
{
    if (plane == rhs.plane)
    {
        if (axis == rhs.axis)
        {
            return type < rhs.type;
        }
        else
        {
            return axis < rhs.axis;
        }
    }
    else
    {
        return plane < rhs.plane;
    }
}

template <bool visibilityTest>
bool KdTree::trace(Ray& ray, TraversalBuffer& traversalStack, Mailboxer& mailboxes, Stats& threadStats) const
{
    threadStats.boxTests++;
    if (!mBounds.intersect(ray))
    {
        return false;
    }

    float minT = ray.minT();
    float maxT = ray.maxT();
    const glm::vec3 invDir = 1.f / ray.dir();
    bool hitPrimitive = false;
    const Node* currentNode = &mNodes[0];
    int traversalStackIdx = -1;

    while (currentNode && ray.maxT() >= minT)
    {
        if (currentNode->isLeaf())
        {
            Node::ConstPrimIterator it = currentNode->beginPrimitives();
            for (; it != currentNode->endPrimitives(); ++it)
            {
                if (!mailboxes.Tested((*it)->id()))
                {
                    threadStats.primitiveTests++;
                    if ((*it)->intersect(ray))
                    {
                        if (visibilityTest)
                        {
                            return true;
                        }

                        hitPrimitive = true;
                    }

                    mailboxes.Mark((*it)->id());
                }
            }

            // Pop node from traversal stack
            if (traversalStackIdx >= 0)
            {
                TraversalState& state = traversalStack[traversalStackIdx--];
                currentNode = state.node;
                minT = state.minT;
                maxT = state.maxT;
            }
            else
            {
                currentNode = nullptr;
            }
        }
        else
        {
            threadStats.boxTests++;

            const uint32_t axis = currentNode->splitAxis();
            const float planeT = (currentNode->splitPlane() - ray.origin()[axis]) * invDir[axis];

            const Node* firstChild = &mNodes[currentNode->lowerChildIdx()];
            const Node* secondChild = &mNodes[currentNode->upperChildIdx()];

            bool belowFirst = ray.origin()[axis] <= currentNode->splitPlane();
            if (!belowFirst)
            {
                std::swap(firstChild, secondChild);
            }

            if (maxT < planeT || planeT < 0.f)
            {
                currentNode = firstChild;
            }
            else if (minT > planeT)
            {
                currentNode = secondChild;
            }
            else
            {
                // Hits both sides, enqueue second child in stack
                TraversalState& state = traversalStack[++traversalStackIdx];
                state.minT = planeT;
                state.maxT = maxT;
                state.node = secondChild;

                maxT = planeT > 0.f ? planeT : maxT;
                currentNode = firstChild;
            }
        }
    }
    
    return hitPrimitive;
}

#endif

