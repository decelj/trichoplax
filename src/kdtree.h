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
        
        Node *mLeft, *mRight;
        AABBox *mBBox;
        bool mIsLeaf;
        std::vector<IPrimitive*> mPrims;
        
        typedef std::vector<IPrimitive*>::const_iterator ConstPrimIterator;
        typedef std::vector<IPrimitive*>::iterator PrimIterator;
    };
    
public:
    explicit KdTree();
    ~KdTree();
    
    void build();
    
    bool trace(Ray& ray, bool firstHit) const;
    
    inline void addPrimitive(IPrimitive *p) { mRoot->mPrims.push_back(p); }
    
private:
    void build(Node* node, unsigned int depth);
    short findSplitAxis(Node* node) const;
    bool trace(const Node* n, Ray& ray, bool firstHit) const;
    void destroy(Node* n);
    
    Node* mRoot;
    unsigned int mMaxDepth, mMinDepth, mMaxPrimsPerNode, mLargeNodes, mTotalNodes;
};

#endif

