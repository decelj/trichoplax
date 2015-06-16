#include <sys/ioctl.h>
#include <iostream>
#include <limits>
#include <algorithm>
#include <iterator>

#include "kdtree.h"
#include "aabbox.h"
#include "common.h"
#include "iprimitive.h"
#include "ray.h"
#include "timer.h"
#include "mailboxer.h"

#define MAX_PRIMS_PER_NODE 10

namespace {
    static const float sMinFloat = -std::numeric_limits<float>::max();
    static const float sMaxFloat = std::numeric_limits<float>::max();
    
    void dump(std::vector<IPrimitive*> prims, short axis)
    {
        std::sort(prims.begin(), prims.end(), [axis](IPrimitive* a, IPrimitive* b)
                  { glm::vec3 all, aur, bll, bur; a->bounds(all, aur); b->bounds(bll, bur); return all[axis] < bll[axis]; });
        glm::vec3 ll, ur;
        for (auto it  = prims.begin(); it != prims.end(); ++it) {
            (*it)->bounds(ll, ur);
            std::cout << ll[axis] << ", " << ur[axis] << std::endl;
        }
    }
}

KdTree::KdTree() : 
    mRoot(new Node()),
    mMaxDepth(0),
    mMinDepth(sMaxFloat),
    mLargeNodes(0),
    mTotalNodes(0),
    mMaxPrimsPerNode(0),
    mTotalNumPrims(0)
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
    mTotalNumPrims = mRoot->mPrims.size();
    //mRoot->mIsLeaf = true;
    build(mRoot, 0);
    
    std::cout << "KdTree: Max depth:                 " << mMaxDepth << std::endl;
    std::cout << "KdTree: Min depth:                 " << mMinDepth << std::endl;
    std::cout << "KdTree: Max primitives per node:   " << mMaxPrimsPerNode << std::endl;
    std::cout << "KdTree: Total nodes:               " << mTotalNodes << std::endl;
    std::cout << "KdTree: Large nodes (> " << MAX_PRIMS_PER_NODE << " prims):  "
        << mLargeNodes << std::endl;
    std::cout << "KdTree: Total primatives:          " << mTotalNumPrims << std::endl;
    std::cout << "KdTree: Build time:                " << t.elapsed()
        << " seconds" << std::endl;
}

void KdTree::build(Node* n, unsigned int depth)
{
    ++mTotalNodes;
    
    // Base case
    if (n->mPrims.size() <= MAX_PRIMS_PER_NODE) {
        //std::cout << "build leaf" << std::endl;
        mMaxDepth = std::max(depth, mMaxDepth);
        mMinDepth = std::min(depth, mMinDepth);
        mMaxPrimsPerNode = std::max(n->mPrims.size(), mMaxPrimsPerNode);
        return;
    }
    
    Node *left = new Node();
    Node *right = new Node();
    
    n->mLeft = left;
    n->mRight = right;
    
    const short axis = findSplitAxis(n);
    const float splitValue = n->mBBox.split(axis);
    
    Node::ConstPrimIterator it = n->mPrims.begin();
    for (; it != n->mPrims.end(); ++it) {
        switch ((*it)->partition(splitValue, axis)) {
            case LEFT:
                left->mPrims.push_back(*it);
                break;
            case RIGHT:
                right->mPrims.push_back(*it);
                break;
            case BOTH:
                right->mPrims.push_back(*it);
                left->mPrims.push_back(*it);
                break;
            default:
                assert(false);
                break;
        }
    }
    
    // If all primitive span the split plane, or they all end up in one node;
    // bail and create a leaf node
    if (right->mPrims.size() == n->mPrims.size() ||
        left->mPrims.size() == n->mPrims.size()) {
        //std::cout << "Depth " << depth << " left " << left->mPrims.size() << " right " << right->mPrims.size() << "; build leaf" << std::endl;
        n->mLeft = NULL;
        n->mRight = NULL;
        
        left->mPrims.clear();
        right->mPrims.clear();
        delete left;
        delete right;
        
        mMaxDepth = std::max(depth, mMaxDepth);
        mMinDepth = std::min(depth, mMinDepth);
        mMaxPrimsPerNode = std::max(n->mPrims.size(), mMaxPrimsPerNode);
        ++mLargeNodes;
        
        return;
    }
    
    //std::cout << "Depth " << depth << " left " << left->mPrims.size() << " right " << right->mPrims.size() << std::endl;
    
    n->mPrims.clear();
    
    left->updateBBox(axis, splitValue, true);
    right->updateBBox(axis, splitValue, false);
    
    build(left, depth+1);
    build(right, depth+1);
}

/* TODO: What was I doing here?? */
bool KdTree::paritionNode(const Node* const node) const
{
    short initialAxis = node->mBBox.longestAxis();
    short axis = initialAxis;
    
    std::vector<IPrimitive*> primsForAxis[3];
    
    do {
        std::copy(node->mPrims.begin(), node->mPrims.end(), std::back_inserter(primsForAxis[axis]));
        
        // Sort primitive BBoxes by axis
        std::sort(primsForAxis[axis].begin(), primsForAxis[axis].end(), [axis](IPrimitive* a, IPrimitive* b)
                  { glm::vec3 all, aur, bll, bur; a->bounds(all, aur); b->bounds(bll, bur); return all[axis] < bll[axis]; });
    } while(axis != initialAxis);
    
    
    return false;
}

short KdTree::findSplitAxis(const Node* const node) const
{
    Node::ConstPrimIterator it;
    unsigned lowestAxis = 0;
    int lowestCost = std::numeric_limits<int>::max();
    
    for (unsigned i = 0; i < 3; ++i)
    {
        int rightNodes = 0, leftNodes = 0, bothNodes = 0;
        const float splitValue = node->mBBox.split(i);
        for (it = node->mPrims.begin(); it != node->mPrims.end(); ++it)
        {
            switch ((*it)->partition(splitValue, i)) {
                case LEFT:
                    ++leftNodes;
                    break;
                    
                case RIGHT:
                    ++rightNodes;
                    break;
                    
                default:
                    ++bothNodes;
                    break;
            }
        }
        
        const int cost = std::abs(leftNodes - rightNodes) + bothNodes;
        if (cost < lowestCost)
        {
            lowestCost = cost;
            lowestAxis = i;
        }
    }
    
    return lowestAxis;
}

bool KdTree::trace(Ray& ray, bool firstHit, Mailboxer& mailboxes) const
{
    if (!mRoot->mBBox.intersect(ray)) return false;
    
    return trace(mRoot, ray, firstHit, mailboxes);
}

bool KdTree::trace(const Node* n, Ray& ray, bool firstHit, Mailboxer& mailboxes) const
{
    bool ret = false;
    
    // Base case
    if (n->isLeaf()) {
        Node::ConstPrimIterator it = n->mPrims.begin();
        for (; it != n->mPrims.end() && !(firstHit && ret); ++it) {
            if (!mailboxes.Tested((*it)->id())) {
                ret |= (*it)->intersect(ray);
                mailboxes.Mark((*it)->id());
            }
        }
    } else {
        if (n->mLeft->mBBox.intersect(ray))
            ret |= trace(n->mLeft, ray, firstHit, mailboxes);
        if (!(ret && firstHit) && n->mRight->mBBox.intersect(ray))
            ret |= trace(n->mRight, ray, firstHit, mailboxes);
    }
    
    return ret;
}

AllignedAllocator<IPrimitive*> KdTree::Node::s_PrimitivePointerAllocator =
    AllignedAllocator<IPrimitive*>(64);

KdTree::Node::Node() :
    mBBox(),
    mLeft(NULL),
    mRight(NULL),
    mPrims(s_PrimitivePointerAllocator)
{ 
    mPrims.clear();
}

KdTree::Node::~Node()
{
    PrimIterator it = mPrims.begin();
    for (; it != mPrims.end(); ++it)
        delete *it;
    mPrims.clear();

    mLeft = mRight = NULL;
}

void KdTree::Node::updateBBox()
{
    glm::vec3 ur(sMinFloat, sMinFloat, sMinFloat),
    ll(sMaxFloat, sMaxFloat, sMaxFloat),
    primUr, primLl;
    
    ConstPrimIterator it = mPrims.begin();
    for (; it != mPrims.end(); ++it) {
        (*it)->bounds(primLl, primUr);
        for (int i = 0; i < 3; i++) {
            ur[i] = std::max(ur[i], primUr[i]);
            ll[i] = std::min(ll[i], primLl[i]);
        }
    }
    
    mBBox.update(ll, ur);
}

void KdTree::Node::updateBBox(const short splitAxis, const float value, bool isLeft)
{
    updateBBox();
    
    
    if (isLeft)
    {
        glm::vec3 ur = mBBox.ur();
        ur[splitAxis] = value;
        mBBox.update(mBBox.ll(), ur);
    }
    else
    {
        glm::vec3 ll = mBBox.ll();
        ll[splitAxis] = value;
        mBBox.update(ll, mBBox.ur());
    }
}
