#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <vector>

class IPrimitive;
class AABBox;
class Ray;

class KdTree
{
    class Node {
    public:
        explicit Node();
        ~Node();
        
        void updateBBox();
        void updateBBox(const short splitAxis, const float value, bool isLeft);
        
        Node *mLeft, *mRight;
        AABBox *mBBox;
        bool mIsLeaf;
        std::vector<IPrimitive*> mPrims;
        
        typedef std::vector<IPrimitive*>::const_iterator ConstPrimIterator;
        typedef std::vector<IPrimitive*>::iterator PrimIterator;
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
    
    bool trace(Ray& ray, bool firstHit, bool* primBuckets) const;
    
    inline void addPrimitive(IPrimitive *p) { mRoot->mPrims.emplace_back(p); }
    inline size_t numberOfPrimitives() const { return mTotalNumPrims; }
    
private:
    void build(Node* node, unsigned int depth);
    bool paritionNode(const Node* const node) const;
    short findSplitAxis(const Node* const node) const;
    bool trace(const Node* n, Ray& ray, bool firstHit, bool* primBuckets) const;
    void destroy(Node* n);
    
    Node* mRoot;
    unsigned int mMaxDepth, mMinDepth, mLargeNodes, mTotalNodes;
    size_t mMaxPrimsPerNode, mTotalNumPrims;
};

#endif

