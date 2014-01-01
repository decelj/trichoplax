#include <sys/ioctl.h>
#include <iostream>
#include <limits>
#include <algorithm>
#include <glm/glm.hpp>
#include <sstream>
#include <string>

#include "kdtree.h"
#include "aabbox.h"
#include "common.h"
#include "iprimitive.h"
#include "ray.h"
#include "timer.h"

#define MAX_PRIMS_PER_NODE 10

namespace {
    static const float sMinFloat = -std::numeric_limits<float>::max();
    
    std::string vec3ToString(const glm::vec3& v) {
        std::stringstream s;
        s << v[0] << ", " << v[1] << ", " << v[2];
        return s.str();
    }
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
    std::cout << "Building KdTree..." << std::endl;
    Timer t;
    t.start();
    mRoot->updateBBox(); // TODO: Move this to after build.
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
    
    // TODO: TMP, unique?
    /*
    for (unsigned int i = 0; i != n->mPrims.size(); ++i)
    {
        for (unsigned int j = i+1; j != n->mPrims.size(); ++j)
            assert(n->mPrims[i] != n->mPrims[j]);
    } */
    
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
    
    // If we can't figure out how to split the node, just
    // make it a leaf.
    if (!splitNode(n, left, right)) {
        n->mIsLeaf = true;
        left->mPrims.clear();
        right->mPrims.clear();
        
        mMaxDepth = MAX(depth, mMaxDepth);
        mMinDepth = MIN(depth, mMinDepth);
        mMaxPrimsPerNode = MAX(n->mPrims.size(), mMaxPrimsPerNode);
        mLargeNodes++;
        
        delete left;
        delete right;
    } else {
        n->mPrims.clear();
        
        assert(left->mPrims.size() > 0);
        assert(right->mPrims.size() > 0);
        
        // TODO: Don't update BBox until after we recurse
        // Improve updateBBox to exapnd node's BBox using child nodes if it's
        // not a leaf node.
        left->updateBBox();
        right->updateBBox();
        
        build(left, depth+1);
        build(right, depth+1);
    }
}

bool KdTree::splitNode(Node* node, Node* left, Node* right) const {
    const short initialAxis = node->mBBox->longestAxis();
    const unsigned int idealSplit = node->mPrims.size() / 2;
    short axis = initialAxis;
    
    do {
        left->mPrims.clear();
        right->mPrims.clear();
        
        _NodeCompare comp(axis);
        std::sort(node->mPrims.begin(), node->mPrims.end(), comp);
        
        KdTree::Node::ConstPrimIterator it = node->mPrims.begin();
        unsigned int i = 0; // TODO: remove
        
        glm::vec3 ll, ur;
        (*(--node->mPrims.end()))->bounds(ll, ur);
        const float maxRightMost = ll[axis];
        (*it)->bounds(ll, ur);
        const float minRightMost = ur[axis];
        float rightMost = ur[axis];
        
        if (minRightMost > maxRightMost) {
            axis = (axis + 1) % 3;
            continue;
        }
        
        for (; it != node->mPrims.end() &&
             (i < idealSplit || ll[axis] < minRightMost);
             ++i, ++it) {
            (*it)->bounds(ll, ur);
            if (ur[axis] > maxRightMost)
                break;
            
            rightMost = MAX(ur[axis], rightMost);
            left->mPrims.push_back(*it);
        }
        
        if (it == node->mPrims.end() || it == node->mPrims.begin()) {
            axis = (axis + 1) % 3;
            continue;
        }
        
        while (it != node->mPrims.end()) {
            (*it)->bounds(ll, ur);
            if (ur[axis] - rightMost > EPSILON || ur[axis] > maxRightMost)
                break;
            
            left->mPrims.push_back(*it);
            ++it;
        }
        
        if (it != node->mPrims.end()) {
            while (it != node->mPrims.end()) {
                (*it)->bounds(ll, ur);
                if (ll[axis] < rightMost)
                    left->mPrims.push_back(*it);
                else
                    right->mPrims.push_back(*it);
                ++it;
            }
            
            if (right->mPrims.size() !=0)
                return true;
        }
        
        axis = (axis + 1) % 3;
    } while (axis != initialAxis);
    
    return false;
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

void KdTree::printSizes(const Node* n)
{
    if (n->mLeft->mIsLeaf)
        std::cout << "Left: " << n->mLeft->mPrims.size() << std::endl;
    if (n->mRight->mIsLeaf)
        std::cout << "Right: " << n->mRight->mPrims.size() << std::endl;
    
    if (!n->mLeft->mIsLeaf)
        printSizes(n->mLeft);
    if (!n->mRight->mIsLeaf)
        printSizes(n->mRight);
    
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

void KdTree::Node::print() const
{
    glm::vec3 ll, ur;
    int i = 0;
    for (auto it = mPrims.begin(); it != mPrims.end(); ++it, ++i)
    {
        (*it)->bounds(ll, ur);
        std::cout << i << " LL: " << vec3ToString(ll) << std::endl;
        std::cout << i << " UR: " << vec3ToString(ur) << std::endl;
    }
}

inline bool KdTree::_NodeCompare::operator()(const IPrimitive* l, const IPrimitive* r)
{
    glm::vec3 lLL, lUR, rLL, rUR;
    l->bounds(lLL, lUR);
    r->bounds(rLL, rUR);
    return lLL[mAxis] < rLL[mAxis];
}

