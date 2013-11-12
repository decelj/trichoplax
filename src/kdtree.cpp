#include <sys/ioctl.h>
#include <iostream>
#include <limits>

#include "kdtree.h"
#include "aabbox.h"
#include "common.h"
#include "iprimitive.h"
#include "ray.h"
#include "timer.h"

#define MAX_PRIMS_PER_NODE 10

namespace {
    static const float sMinFloat = -std::numeric_limits<float>::max();
}

KdTree::KdTree() : 
    mRoot(new Node()),
    mMaxDepth(0),
    mMinDepth(std::numeric_limits<unsigned int>::max()),
    mMaxPrimsPerNode(0)
{
}

KdTree::~KdTree()
{
    destroy(mRoot);
}

void KdTree::destroy(Node* n)
{
    if (n == NULL)
        return;
    
    destroy(n->mLeft);
    destroy(n->mRight);
    
    delete n;
}

void KdTree::build()
{
    Timer t;
    t.start();
    mRoot->updateBBox();
    build(mRoot, 0);
    std::cout << "KdTree: Max depth:                 " << mMaxDepth << std::endl;
    std::cout << "KdTree: Min depth:                 " << mMinDepth << std::endl;
    std::cout << "KdTree: Max primitives per node:   " << mMaxPrimsPerNode << std::endl;
    std::cout << "KdTree: Total nodes:               " << mTotalNodes << std::endl;
    std::cout << "KdTree: Large nodes (> " << MAX_PRIMS_PER_NODE << " prims):  "
        << mLargeNodes << std::endl;
    std::cout << "KdTree: Build time:                " << t.elapsed()
        << " seconds" << std::endl;
}

void KdTree::build(Node* n, unsigned int depth)
{
    mTotalNodes++;
    
    // Base case
    if (n->mPrims.size() <= MAX_PRIMS_PER_NODE) {
        n->mIsLeaf = true;
        mMaxDepth = MAX(depth, mMaxDepth);
        mMinDepth = MIN(depth, mMinDepth);
        mMaxPrimsPerNode = MAX(n->mPrims.size(), mMaxPrimsPerNode);
        return;
    }
    
    Node *left = new Node();
    Node *right = new Node();
    
    n->mLeft = left;
    n->mRight = right;
    
    const short axis = findSplitAxis(n);
    const float splitValue = n->mBBox->split(axis);
    
    Node::ConstPrimIterator it = n->mPrims.begin();
    for (; it != n->mPrims.end(); ++it) {
        if ((*it)->onLeftOfPlane(splitValue, axis))
            left->mPrims.push_back(*it);
        else
            right->mPrims.push_back(*it);
    }
    
    // All primitives span the split plane and/or are on the right
    if (right->mPrims.size() == 0 || left->mPrims.size() == 0) {
        n->mIsLeaf = true;
        left->mPrims.clear();
        right->mPrims.clear();
        
        mMaxDepth = MAX(depth, mMaxDepth);
        mMinDepth = MIN(depth, mMinDepth);
        mMaxPrimsPerNode = MAX(n->mPrims.size(), mMaxPrimsPerNode);
        mLargeNodes++;
        
        delete left;
        delete right;
        return;
    }
    
    n->mPrims.clear();
    
    left->updateBBox();
    right->updateBBox();
    
    build(left, depth+1);
    build(right, depth+1);
}

short KdTree::findSplitAxis(Node* node) const
{
    short initialAxis = node->mBBox->longestAxis();
    short axis = initialAxis;
    float splitValue = node->mBBox->split(axis);
    Node::ConstPrimIterator it;
    do {
        it = node->mPrims.begin();
        while (it != node->mPrims.end() && !(*it)->onLeftOfPlane(splitValue, axis))
            ++it;
        
        if (it != node->mPrims.end())
            break;
        
        axis = (axis + 1) % 3;
        splitValue = node->mBBox->split(axis);
    } while (axis != initialAxis);
    
    return axis;
}

bool KdTree::trace(Ray& ray, bool firstHit) const
{
    if (!mRoot->mBBox->intersect(ray)) return false;
    return trace(mRoot, ray, firstHit);
}

bool KdTree::trace(const Node* n, Ray& ray, bool firstHit) const
{
    bool ret = false;
    
    // Base case
    if (n->mIsLeaf) {
        Node::ConstPrimIterator it = n->mPrims.begin();
        for (; it != n->mPrims.end(); ++it) {
            if (firstHit && ret) break;
            ret |= (*it)->intersect(ray);
        }
    } else {
        /* TODO:
         If the ray origin is outside of the BBox, and the distance of the ray
         to the closest point on the box is greater than the minDist, we can
         skip that entier branch of the tree.
         */
        if (n->mLeft->mBBox->intersect(ray))
            ret |= trace(n->mLeft, ray, firstHit);
        if (!(ret && firstHit) && n->mRight->mBBox->intersect(ray))
            ret |= trace(n->mRight, ray, firstHit);
    }
    
    return ret;
}

KdTree::Node::Node() :
mLeft(NULL), mRight(NULL), mBBox(new AABBox()), mIsLeaf(false) 
{ 
    mPrims.clear();
}

KdTree::Node::~Node()
{
    PrimIterator it = mPrims.begin();
    for (; it != mPrims.end(); ++it)
        delete *it;
    mPrims.clear();
    delete mBBox;
}

void KdTree::Node::updateBBox()
{
    glm::vec3 ur(sMinFloat, sMinFloat, sMinFloat), ll(MAXFLOAT, MAXFLOAT, MAXFLOAT), primUr, primLl;
    ConstPrimIterator it = mPrims.begin();
    for (; it != mPrims.end(); ++it) {
        (*it)->bounds(primLl, primUr);
        for (int i = 0; i < 3; i++) {
            ur[i] = MAX(ur[i], primUr[i]);
            ll[i] = MIN(ll[i], primLl[i]);
        }
    }
    
    mBBox->update(ll, ur);
}
