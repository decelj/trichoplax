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
    class Node {
    private:
        typedef std::vector<const IPrimitive*, AlignedAllocator<const IPrimitive*> > PrimitiveVector;
        
    public:
        typedef PrimitiveVector::const_iterator ConstPrimIterator;
        typedef PrimitiveVector::iterator PrimIterator;

        explicit Node();
        ~Node();
        
        void updateBBox();
        void updateBBox(const unsigned splitAxis, const float value, bool isLeft);
        bool isLeaf() const { return right == NULL; }
        
        AABBox BBox;            // 24 bytes
        Node *left;             // 8 bytes
        Node *right;            // 8 bytes
        PrimitiveVector prims;  // 8 bytes
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
    explicit KdTree();
    ~KdTree();
    
    void build();
    
    bool trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const;
    
    void addPrimitive(const IPrimitive* p);
    size_t numberOfPrimitives() const { return mTotalNumPrims; }
    void printTraversalStats(double raysCast) const;
    
private:
    void build(Node* node, SHAPlaneEventList& events, unsigned int depth);
    bool trace(const Node* n, Ray& ray, bool firstHit, Mailboxer& mailboxes) const;

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
    TP_ASSERT(mRoot->left == NULL);
    mRoot->prims.emplace_back(p);
}

inline bool KdTree::trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const
{
    mTraversalStats.incrementBoxTests();
    if (!mRoot->BBox.intersect(ray)) return false;
    
    return trace(mRoot, ray, firstHit, mailboxes);
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

