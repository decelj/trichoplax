#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>
#include <list>

#include "aligned_allocator.h"
#include "aabbox.h"
#include "common.h"

#include <atomic>

class Mailboxer;
class IPrimitive;
class Ray;


class KdTree
{
private:
    class Node
    {
    private:
        typedef std::vector<const IPrimitive*, AlignedAllocator<const IPrimitive*> > PrimitiveVector;
        
    public:
        typedef PrimitiveVector::const_iterator ConstPrimIterator;
        typedef PrimitiveVector::iterator PrimIterator;

        explicit Node();
        ~Node();

        inline void addPrimitive(const IPrimitive* prim) { mPrims.emplace_back(prim); }
        inline void clearPrimitves() { mPrims.clear(); }
        void updateBounds();
        void split(const unsigned splitAxis, const float plane);

        inline Node* left() { return mLeft; }
        inline Node* right() { return mRight; }
        inline const Node& left() const { return *mLeft; }
        inline const Node& right() const { return *mRight; }

        inline bool isLeaf() const { return mLeft == nullptr; }
        inline const AABBox& bounds() const { return mBBox; }
        inline size_t primitiveCount() const { return mPrims.size(); }
        inline bool intersectBounds(const Ray& ray) const { return mBBox.intersect(ray); }

        inline ConstPrimIterator beginPrimitives() const { return mPrims.begin(); }
        inline ConstPrimIterator endPrimitives() const { return mPrims.end(); }

    private:
        AABBox mBBox;            // 48 bytes
        Node* mLeft;             // 8 bytes
        Node* mRight;            // 8 bytes
        PrimitiveVector mPrims;  // 8 bytes
    };
    
    enum SHAPlaneEventType
    {
        END = 0,
        PLANAR,
        START
    };
    
    struct SHAPlaneEvent
    {
        SHAPlaneEvent(const IPrimitive* _prim, float _plane, unsigned _axis,
                      SHAPlaneEventType _type);
        
        bool operator<(const SHAPlaneEvent& rhs) const;
        
        const IPrimitive* primitive;
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

    struct TraversalStats
    {
        TraversalStats();

        void incrementBoxTests();
        void incrementPrimitiveTests();

        std::atomic_ullong boxTests;
        std::atomic_ullong primitiveTests;
    };
    
public:
    typedef std::vector<const Node*, AlignedAllocator<const Node*> > TraversalBuffer;

    explicit KdTree();
    ~KdTree();
    
    void build();
    
    bool trace(Ray& ray, bool firstHit, TraversalBuffer& traversalStack, Mailboxer& mailboxes) const;
    
    void addPrimitive(const IPrimitive* p);
    size_t numberOfPrimitives() const { return mTotalNumPrims; }
    void printTraversalStats(double raysCast) const;
    TraversalBuffer allocateTraversalBuffer() const;
    
private:
    void build(Node* node, SHAPlaneEventList& events, unsigned int depth);

    void split(SHAPlaneEventList& outLeftEvents, SHAPlaneEventList& outRightEvents,
               Node& node, const SHASplitPlane& plane, SHAPlaneEventList& events) const;
    SHASplitPlane findSplitPlane(float* outCost, const AABBox& voxel,
                                 const SHAPlaneEventList& events, unsigned totalNumPrimitives) const;
    void SHACost(float* lowestCostOut, SHASplitPlane::Side* outSide,
                 float plane, unsigned aaAxis, const AABBox& voxel,
                 unsigned numLeftPrims, unsigned numRightPrims, unsigned numPlanarPrims) const;

    void generateEventsForPrimitive(const IPrimitive* primitive, const AABBox& voxel,
                                    SHAPlaneEventList& events) const;
    void DumpSplitEvents(SHAPlaneEventList::const_iterator begin,
                         SHAPlaneEventList::const_iterator end, unsigned aaAxis=4) const;
    
    Node*                   mRoot;
    mutable TraversalStats  mTraversalStats;
    unsigned                mMaxDepth;
    unsigned                mMinDepth;
    size_t                  mLargeNodes;
    size_t                  mLeafNodes;
    size_t                  mTotalNodes;
    size_t                  mMaxPrimsPerNode;
    size_t                  mTotalNumPrims;
};


inline void KdTree::addPrimitive(const IPrimitive* p)
{
    TP_ASSERT(mRoot->left() == nullptr);
    mRoot->addPrimitive(p);
}

inline bool KdTree::SHAPlaneEvent::operator<(const SHAPlaneEvent& rhs) const
{
    if (plane == rhs.plane)
    {
        return type < rhs.type;
    }
    else
    {
        return plane < rhs.plane;
    }
}

inline void KdTree::TraversalStats::incrementBoxTests()
{
    boxTests.fetch_add(1, std::memory_order_relaxed);
}

inline void KdTree::TraversalStats::incrementPrimitiveTests()
{
    primitiveTests.fetch_add(1, std::memory_order_relaxed);
}

#endif

