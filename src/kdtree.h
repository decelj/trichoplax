#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>
#include <list>

#include "aligned_allocator.h"
#include "aabbox.h"
#include "common.h"

#include <cstdint>

class Mailboxer;
class Triangle;
class Ray;
class Stats;


class KdTree
{
private:
    class Node
    {
    private:
        using PrimitiveVector = std::vector<const Triangle*, AlignedAllocator<const Triangle*> >;
        
    public:
        typedef PrimitiveVector::const_iterator ConstPrimIterator;
        typedef PrimitiveVector::iterator PrimIterator;
        static const uint32_t sInvalidNodeIdx;

        explicit Node();
        ~Node();

        void addPrimitive(const Triangle* prim) { mPrims.emplace_back(prim); }
        void clearPrimitves() { mPrims.clear(); }
        bool isEmpty() { return mPrims.empty(); }
        void updateBounds();

        void setChildren(uint32_t left, uint32_t right);
        uint32_t leftIdx() const { return mLeft; }
        uint32_t rightIdx() const { return mRight; }

        bool isLeaf() const { return mLeft == sInvalidNodeIdx && mRight == sInvalidNodeIdx; }
        size_t primitiveCount() const { return mPrims.size(); }
        bool intersectBounds(const Ray& ray) const { return bounds.intersect(ray); }

        ConstPrimIterator beginPrimitives() const { return mPrims.begin(); }
        ConstPrimIterator endPrimitives() const { return mPrims.end(); }

        AABBox bounds;           // 24 bytes
    private:
        uint32_t mLeft;          // 4 bytes
        uint32_t mRight;         // 4 bytes
        PrimitiveVector mPrims;  // 8 bytes
    };
    using NodeBuffer = std::vector<Node, AlignedAllocator<Node> >;

    class NodePtr
    {
    public:
        NodePtr(uint32_t nodeIdx, NodeBuffer& nodes)
        : mNodes(&nodes), mNodeIdx(nodeIdx)
        { }

        NodePtr()
        : mNodes(nullptr), mNodeIdx(Node::sInvalidNodeIdx)
        { }

        operator bool() const { return mNodeIdx != Node::sInvalidNodeIdx; }

        Node* operator->() { return &(*mNodes)[mNodeIdx]; }
        const Node* operator->() const { return &(*mNodes)[mNodeIdx]; }

    private:
        NodeBuffer* mNodes;
        const uint32_t mNodeIdx;
    };
    
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
    
    typedef std::list<SHAPlaneEvent> SHAPlaneEventList;
    
    struct SHASplitPlane
    {
        enum Side
        {
            LEFT = 0,
            RIGHT
        };
        
        float plane;
        unsigned aaAxis;
        Side side;
    };
    
public:
    using TraversalBuffer = std::vector<uint32_t, AlignedAllocator<uint32_t> >;

    explicit KdTree();
    ~KdTree();
    
    void build();
    
    bool trace(Ray& ray, bool firstHit, TraversalBuffer& traversalStack, Mailboxer& mailboxes, Stats& threadStats) const;
    
    void addPrimitive(const Triangle* p);
    size_t numberOfPrimitives() const { return mTotalNumPrims; }
    TraversalBuffer allocateTraversalBuffer() const;
    
private:
    Node& root() { return mNodes[0]; }

    void build(NodePtr node, SHAPlaneEventList& events, unsigned int depth, uint32_t* nextNodeIdx);

    void split(SHAPlaneEventList& outLeftEvents, SHAPlaneEventList& outRightEvents,
               NodePtr node, const SHASplitPlane& plane, SHAPlaneEventList& events,
               uint32_t* nextNodeIdx);
    SHASplitPlane findSplitPlane(float* outCost, const AABBox& voxel,
                                 const SHAPlaneEventList& events, unsigned totalNumPrimitives) const;
    void SHACost(float* lowestCostOut, SHASplitPlane::Side* outSide,
                 float plane, unsigned aaAxis, const AABBox& voxel,
                 unsigned numLeftPrims, unsigned numRightPrims, unsigned numPlanarPrims) const;

    void generateEventsForPrimitive(const Triangle* primitive, const AABBox& voxel,
                                    SHAPlaneEventList& events) const;
    void DumpSplitEvents(SHAPlaneEventList::const_iterator begin,
                         SHAPlaneEventList::const_iterator end, unsigned aaAxis=4) const;

    NodeBuffer              mNodes;
    unsigned                mMaxDepth;
    unsigned                mMinDepth;
    size_t                  mLargeNodes;
    size_t                  mLeafNodes;
    size_t                  mTotalNodes;
    size_t                  mMaxPrimsPerNode;
    size_t                  mTotalNumPrims;
};


inline void KdTree::addPrimitive(const Triangle* p)
{
    TP_ASSERT(root().isLeaf());
    root().addPrimitive(p);
}

inline void KdTree::Node::setChildren(uint32_t left, uint32_t right)
{
    TP_ASSERT(left != right);
    mLeft = left;
    mRight = right;
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

#endif

