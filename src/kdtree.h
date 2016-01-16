#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>
#include <list>

#include "aligned_allocator.h"
#include "aabbox.h"
#include "common.h"

class Mailboxer;
class IPrimitive;
class Ray;

class KdTree
{
    class Node {
        typedef std::vector<const IPrimitive*, AlignedAllocator<const IPrimitive*> > PrimitiveVector;
        
    public:
        explicit Node();
        ~Node();
        
        void updateBBox();
        void updateBBox(const unsigned splitAxis, const float value, bool isLeft);
        inline bool isLeaf() const { return !mRight; }
        
        AABBox mBBox;
        Node *mLeft, *mRight;
        PrimitiveVector mPrims;
        
        typedef PrimitiveVector::const_iterator ConstPrimIterator;
        typedef PrimitiveVector::iterator PrimIterator;
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
    
public:
    explicit KdTree();
    ~KdTree();
    
    void build();
    
    bool trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const;
    
    void addPrimitive(const IPrimitive* p);
    size_t numberOfPrimitives() const { return mTotalNumPrims; }
    
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
    
    Node*           mRoot;
    unsigned        mMaxDepth;
    unsigned        mMinDepth;
    size_t          mLargeNodes;
    size_t          mLeafNodes;
    size_t          mTotalNodes;
    size_t          mMaxPrimsPerNode;
    size_t          mTotalNumPrims;
};


inline void KdTree::addPrimitive(const IPrimitive* p)
{
    TP_ASSERT(mRoot->mLeft == NULL);
    mRoot->mPrims.emplace_back(p);
}

inline bool KdTree::trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const
{
    if (!mRoot->mBBox.intersect(ray)) return false;
    
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

#endif

