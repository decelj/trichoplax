#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>
#include "alligned_allocator.h"
#include "aabbox.h"

class Mailboxer;
class IPrimitive;
class Ray;

class KdTree
{
    class Node {
        typedef std::vector<IPrimitive*, AllignedAllocator<IPrimitive*> > PrimitiveVector;
        
        static AllignedAllocator<IPrimitive*> s_PrimitivePointerAllocator;
        
    public:
        explicit Node();
        ~Node();
        
        void updateBBox();
        void updateBBox(const short splitAxis, const float value, bool isLeft);
        inline bool isLeaf() const { return !mRight; }
        
        AABBox mBBox;
        Node *mLeft, *mRight;
        PrimitiveVector mPrims;
        
        typedef PrimitiveVector::const_iterator ConstPrimIterator;
        typedef PrimitiveVector::iterator PrimIterator;
    };
    
public:
    enum PartitionResult {
        LEFT = 0,
        RIGHT,
        BOTH
    };
    
    explicit KdTree();
    ~KdTree();
    
    void build();
    
    bool trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const;
    
    inline void addPrimitive(IPrimitive *p) { mRoot->mPrims.emplace_back(p); }
    inline size_t numberOfPrimitives() const { return mTotalNumPrims; }
    
private:
    void build(Node* node, unsigned int depth);
    bool paritionNode(const Node* const node) const;
    short findSplitAxis(const Node* const node) const;
    bool trace(const Node* n, Ray& ray, bool firstHit, Mailboxer& mailboxes) const;
    void destroy(Node* n);
    
    Node* mRoot;
    unsigned int mMaxDepth, mMinDepth, mLargeNodes, mTotalNodes;
    size_t mMaxPrimsPerNode, mTotalNumPrims;
};

#endif

