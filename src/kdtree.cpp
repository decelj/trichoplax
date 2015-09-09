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
    mLeafNodes(0),
    mTotalNodes(0),
    mMaxPrimsPerNode(0),
    mTotalNumPrims(0)
{
}

KdTree::~KdTree()
{
    delete mRoot;
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
    std::cout << "KdTree: Leaf nodes:                " << mLeafNodes << std::endl;
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
        ++mLeafNodes;
        return;
    }
    
    Node *left = new Node();
    Node *right = new Node();
    
    n->mLeft = left;
    n->mRight = right;
    
    unsigned axis;
    float splitValue;
    split(n, &axis, &splitValue);
    
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
        ++mLeafNodes;
        
        return;
    }
    
    //std::cout << "Depth " << depth << " left " << left->mPrims.size() << " right " << right->mPrims.size() << std::endl;
    
    n->mPrims.clear();
    
    left->updateBBox(axis, splitValue, true);
    right->updateBBox(axis, splitValue, false);
    
    build(left, depth+1);
    build(right, depth+1);
}

void KdTree::split(Node* const node, unsigned* axis, float* value) const
{
    Node::ConstPrimIterator it;
    int lowestCost = std::numeric_limits<int>::max();
    
    for (unsigned i = 0; i < 3; ++i)
    {
        /* Sort primitives on selected axis */
        std::sort(node->mPrims.begin(), node->mPrims.end(), [i](IPrimitive* a, IPrimitive* b)
                  { glm::vec3 all, aur, bll, bur; a->bounds(all, aur); b->bounds(bll, bur); return all[i] < bll[i]; });
        
        /* Set split plane to middle primitive's lower left value */
        glm::vec3 ll, ur;
        unsigned middleIdx = node->mPrims.size() / 2;
        node->mPrims[middleIdx]->bounds(ll, ur);
        const float splitValue = ll[i];
        
        int rightNodes = 0, leftNodes = 0, bothNodes = 0;
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
        
        const unsigned cost = std::abs(leftNodes - rightNodes) + bothNodes;
        if (cost < lowestCost)
        {
            lowestCost = cost;
            *axis = i;
            *value = splitValue;
        }
    }
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

KdTree::Node::Node() :
    mBBox(),
    mLeft(NULL),
    mRight(NULL),
    mPrims()
{
    mPrims.clear();
}

KdTree::Node::~Node()
{
    PrimIterator it = mPrims.begin();
    for (; it != mPrims.end(); ++it)
        delete *it;
    
    mPrims.clear();

    delete mLeft;
    delete mRight;
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

void KdTree::Node::updateBBox(const unsigned splitAxis, const float value, bool isLeft)
{
    updateBBox();
    
    /* Clip bounding box to selected split */
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
