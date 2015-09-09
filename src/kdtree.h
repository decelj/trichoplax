#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>
#include "aligned_allocator.h"
#include "aabbox.h"

class Mailboxer;
class IPrimitive;
class Ray;

class KdTree
{
    class Node {
        typedef std::vector<IPrimitive*, AlignedAllocator<IPrimitive*> > PrimitiveVector;
        
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
    void split(Node* const node, unsigned* axis, float* value) const;
    bool trace(const Node* n, Ray& ray, bool firstHit, Mailboxer& mailboxes) const;
    
    Node* mRoot;
    unsigned int mMaxDepth, mMinDepth;
    size_t mLargeNodes, mLeafNodes, mTotalNodes;
    size_t mMaxPrimsPerNode, mTotalNumPrims;
};

inline bool KdTree::trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const
{
    if (!mRoot->mBBox.intersect(ray)) return false;
    
    return trace(mRoot, ray, firstHit, mailboxes);
}

#endif

