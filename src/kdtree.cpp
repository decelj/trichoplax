#include <sys/ioctl.h>
#include <iostream>
#include <limits>
#include <algorithm>
#include <iterator>
#include <unordered_map>

#include "kdtree.h"
#include "aabbox.h"
#include "common.h"
#include "iprimitive.h"
#include "ray.h"
#include "timer.h"
#include "mailboxer.h"

#define MAX_PRIMS_PER_NODE 10

namespace
{
static const float sMinFloat = -std::numeric_limits<float>::max();
static const float sMaxFloat = std::numeric_limits<float>::max();
static const float sTraversalCost = 15.f;
static const float sIntersectionConst = 20.f;

void dump(std::vector<IPrimitive*, AlignedAllocator<IPrimitive*> > prims, unsigned axis)
{
    std::sort(prims.begin(), prims.end(), [axis](IPrimitive* a, IPrimitive* b)
              { glm::vec3 all, aur, bll, bur; a->bounds(all, aur); b->bounds(bll, bur); return all[axis] < bll[axis]; });
    glm::vec3 ll, ur;
    for (auto it  = prims.begin(); it != prims.end(); ++it) {
        (*it)->bounds(ll, ur);
        std::cout << "(" << (*it)->id() << ") " << ll[axis] << ", " << ur[axis] << std::endl;
    }
}
    
inline float calculateSplitCost(float probabilityHitLeft, float probabilityHitRight, unsigned numPrimsLeft, unsigned numPrimsRight)
{
    //float modifer = numPrimsLeft == 0 || numPrimsRight == 0 ? .8f : 1.f;
    float cost = sIntersectionConst * (probabilityHitLeft * (float)numPrimsLeft + probabilityHitRight * (float)numPrimsRight);
    cost += sTraversalCost;
    //cost *= modifer;

    return cost;
}
} // annonymous namspace

KdTree::KdTree() : 
    mRoot(new Node()),
    mMaxDepth(0),
    mMinDepth(std::numeric_limits<unsigned>::max()),
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
    
    SHAPlaneEventList initialEvents;
    Node::PrimIterator it = mRoot->mPrims.begin();
    for (; it != mRoot->mPrims.end(); ++it)
    {
        generateEventsForPrimitive(*it, mRoot->mBBox, initialEvents);
    }
    
    initialEvents.sort();
    
#if 0
    DumpSplitEvents(initialEvents.begin(), initialEvents.end());
    exit(0);
#endif
    
    //mRoot->mIsLeaf = true;
    build(mRoot, initialEvents, 0);
    
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

void KdTree::build(Node* node, SHAPlaneEventList& events, unsigned int depth)
{
    ++mTotalNodes;

    const float leafCost = sIntersectionConst * node->mPrims.size();
    float splitCost = 0.f;
    SHASplitPlane splitPlane = findSplitPlane(&splitCost, node->mBBox, events, node->mPrims.size());

    // Base case
    if (leafCost < splitCost) {
        //std::cout << "build leaf" << std::endl;
        mMaxDepth = std::max(depth, mMaxDepth);
        mMinDepth = std::min(depth, mMinDepth);
        mMaxPrimsPerNode = std::max(node->mPrims.size(), mMaxPrimsPerNode);
        ++mLeafNodes;
    }
    else
    {
        SHAPlaneEventList leftEvents, rightEvents;
        split(leftEvents, rightEvents, *node, splitPlane, events);
        events.clear();
        
        build(node->mLeft, leftEvents, depth+1);
        build(node->mRight, rightEvents, depth+1);
    }
}

void KdTree::split(SHAPlaneEventList& outLeftEvents, SHAPlaneEventList& outRightEvents, Node& node, const SHASplitPlane& plane, SHAPlaneEventList& events) const
{
    enum PrimSide
    {
        PS_LEFT = 0,
        PS_RIGHT,
        PS_BOTH
    };
    
    typedef std::unordered_map<IPrimitive*, PrimSide> PrimSideMap;

#if 0
    std::cout << "Plane: " << plane.plane << "(" << plane.aaAxis << ")" << std::endl;
    DumpSplitEvents(events.begin(), events.end(), plane.aaAxis);
#endif
    
    /* Classify the primitives based on the split plane and the split events */
    PrimSideMap primClassification;
    for (Node::PrimIterator it = node.mPrims.begin(); it != node.mPrims.end(); ++it)
        primClassification[*it] = PS_BOTH;

    for (SHAPlaneEventList::const_iterator it = events.begin(); it != events.end(); ++it)
    {
        if (it->type == END && it->axis == plane.aaAxis && it->plane <= plane.plane)
        {
            primClassification[it->primitive] = PS_LEFT;
            //std::cout << "(" << it->primitive->id() << ") LEFT" << std::endl;
        }
        else if (it->type == START && it->axis == plane.aaAxis && it->plane >= plane.plane)
        {
            primClassification[it->primitive] = PS_RIGHT;
            //std::cout << "(" << it->primitive->id() << ") RIGHT" << std::endl;
        }
        else if (it->type == PLANAR && it->axis == plane.aaAxis)
        {
            if (it->plane < plane.plane ||
                (it->plane == plane.plane && plane.side == SHASplitPlane::LEFT))
            {
                primClassification[it->primitive] = PS_LEFT;
                //std::cout << "(" << it->primitive->id() << ") Coplanar LEFT" << std::endl;
            }
            else if (it->plane > plane.plane ||
                     (it->plane == plane.plane && plane.side == SHASplitPlane::RIGHT))
            {
                primClassification[it->primitive] = PS_RIGHT;
                //std::cout << "(" << it->primitive->id() << ") Coplanar RIGHT" << std::endl;
            }
        }
    }

    //std::cout << std::endl;
    
    /* Split the node */
    node.mPrims.clear();
    node.mLeft = new Node;
    node.mRight = new Node;
    for (PrimSideMap::const_iterator it = primClassification.begin(); it != primClassification.end(); ++it)
    {
        switch (it->second) {
            case PS_LEFT:
                node.mLeft->mPrims.push_back(it->first);
                break;
                
            case PS_RIGHT:
                node.mRight->mPrims.push_back(it->first);
                break;
                
            case PS_BOTH:
            default:
                node.mLeft->mPrims.push_back(it->first);
                node.mRight->mPrims.push_back(it->first);
                break;
        }
    }

#if 0
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << plane.aaAxis << ", " << plane.plane << std::endl;
    std::cout << "Left (" << node.mLeft->mPrims.size() << "):" << std::endl;
    dump(node.mLeft->mPrims, plane.aaAxis);
    std::cout << std::endl << "Right (" << node.mRight->mPrims.size() << "):" << std::endl;
    dump(node.mRight->mPrims, plane.aaAxis);
    std::cout << "--------------------------------------------------" << std::endl << std::endl;
#endif

    node.mBBox.split(&node.mLeft->mBBox, &node.mRight->mBBox, plane.plane, plane.aaAxis);
    
    /* Generate SHAPlaneEvents for left and right nodes */
    {
        SHAPlaneEventList newLeftEvents;
        SHAPlaneEventList newRightEvents;
        
        SHAPlaneEventList::iterator it = events.begin();
        while (it != events.end())
        {
            PrimSideMap::const_iterator primIt = primClassification.find(it->primitive);
            if (primIt != primClassification.end())
            {
                switch (primIt->second) {
                case PS_LEFT:
                    TP_ASSERT(
                        std::find(node.mRight->mPrims.begin(), node.mRight->mPrims.end(), primIt->first)
                            == node.mRight->mPrims.end());
                    outLeftEvents.emplace_back(*it);
                    break;
                    
                case PS_RIGHT:
                    TP_ASSERT(
                        std::find(node.mLeft->mPrims.begin(), node.mLeft->mPrims.end(), primIt->first)
                            == node.mLeft->mPrims.end());
                    outRightEvents.emplace_back(*it);
                    break;
                    
                case PS_BOTH:
                    TP_ASSERT(
                      std::find(node.mLeft->mPrims.begin(), node.mLeft->mPrims.end(), primIt->first)
                          != node.mLeft->mPrims.end());
                    TP_ASSERT(
                      std::find(node.mRight->mPrims.begin(), node.mRight->mPrims.end(), primIt->first)
                          != node.mRight->mPrims.end());
                    generateEventsForPrimitive(it->primitive, node.mLeft->mBBox, newLeftEvents);
                    generateEventsForPrimitive(it->primitive, node.mRight->mBBox, newRightEvents);
                    primClassification.erase(primIt);
                    break;

                default:
                    break;
                }
            }
            
            ++it;
        }
        
        newLeftEvents.sort();
        newRightEvents.sort();
        
        it = outLeftEvents.begin();
        SHAPlaneEventList::iterator newEventsIt = newLeftEvents.begin();
        for (; newEventsIt != newLeftEvents.end(); ++newEventsIt)
        {
            while (it != outLeftEvents.end() && *it < *newEventsIt) {
                ++it;
            }
            
            outLeftEvents.insert(it, *newEventsIt);
        }
        
        it = outRightEvents.begin();
        newEventsIt = newRightEvents.begin();
        for (; newEventsIt != newRightEvents.end(); ++newEventsIt)
        {
            while (it != outRightEvents.end() && *it < *newEventsIt) {
                ++it;
            }
            
            outRightEvents.insert(it, *newEventsIt);
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

void KdTree::generateEventsForPrimitive(IPrimitive* primitive, const AABBox& voxel, SHAPlaneEventList& events) const
{
    glm::vec3 primLL, primUR;
    primitive->bounds(primLL, primUR);
    const AABBox primBox(primLL, primUR);
    const AABBox clippedBox = voxel.intersection(primBox);
    TP_ASSERT(clippedBox.isValid());
    
    // Clipped box width, height, depth
    const glm::vec3 clippedBoxWHD = clippedBox.ur() - clippedBox.ll();
    
    for (unsigned axis = 0; axis < 3; ++axis)
    {
        if (relEq(clippedBox.ll()[axis], clippedBox.ur()[axis]) ||
            primitive->isCoplaner(clippedBox.ll()[axis], axis))
        {
            // Area of the voxel face which is parallel to the clip plane
            const float area = clippedBoxWHD[(axis + 1) % 3] * clippedBoxWHD[(axis + 2) % 3];

            // Check for point/line intersection with voxel
            TP_ASSERT(!relEq(area,  0.f));
            events.emplace_back(primitive, clippedBox.ll()[axis], axis, PLANAR);
        }
        else
        {
            // Sum of the areas of the voxel faces which are orthognal to the
            // clip plane
            const float area = clippedBoxWHD[axis] * clippedBoxWHD[(axis + 1) % 3]
                + clippedBoxWHD[axis] * clippedBoxWHD[(axis + 2) % 3];

            // Check for point/line intersection with voxel
            TP_ASSERT(!relEq(area,  0.f));
            events.emplace_back(primitive, clippedBox.ll()[axis], axis, START);
            events.emplace_back(primitive, clippedBox.ur()[axis], axis, END);
        }
    }
}

KdTree::SHASplitPlane KdTree::findSplitPlane(float* lowestCostOut,
    const AABBox& voxel, const SHAPlaneEventList& events,
    unsigned totalNumPrimitives) const
{
    *lowestCostOut = std::numeric_limits<float>::max();
    unsigned numLeft[3] = {0, 0, 0};
    unsigned numPlanar[3] = {0, 0, 0};
    unsigned numRight[3] = {totalNumPrimitives, totalNumPrimitives, totalNumPrimitives};
    SHASplitPlane bestSplitPlane;

#if DEBUG
    unsigned lowestCounts[3] = {0, 0, 0};
#endif
    
    SHAPlaneEventList::const_iterator it = events.begin();
    while (it != events.end())
    {
        float plane = it->plane;
        unsigned aaAxis = it->axis;
        unsigned startingAtPlane = 0;
        unsigned planar = 0;
        unsigned endingAtPlane = 0;
        
        while (it != events.end() && it->axis == aaAxis && it->plane == plane && it->type == END)
        {
            ++endingAtPlane;
            ++it;
        }
        
        while (it != events.end() && it->axis == aaAxis && it->plane == plane && it->type == PLANAR)
        {
            ++planar;
            ++it;
        }
        
        while (it != events.end() && it->axis == aaAxis && it->plane == plane && it->type == START)
        {
            ++startingAtPlane;
            ++it;
        }
        
        numPlanar[aaAxis] = planar;
        numRight[aaAxis] -= planar;
        numRight[aaAxis] -= endingAtPlane;
        
        float cost;
        SHASplitPlane::Side side;
        SHACost(&cost, &side, plane, aaAxis, voxel, numLeft[aaAxis], numRight[aaAxis], numPlanar[aaAxis]);
        
        if (cost < *lowestCostOut)
        {
            bestSplitPlane.plane = plane;
            bestSplitPlane.aaAxis = aaAxis;
            bestSplitPlane.side = side;
            *lowestCostOut = cost;

#if DEBUG
            lowestCounts[0] = numLeft[aaAxis];
            lowestCounts[1] = numRight[aaAxis];
            lowestCounts[2] = numPlanar[aaAxis];
#endif
        }
        
        numLeft[aaAxis] += startingAtPlane;
        numLeft[aaAxis] += planar;
    }

    //std::cout << "Left: " << lowestCounts[0] << " Right: " << lowestCounts[1] << " Planar(" << (bestSplitPlane.side == SHASplitPlane::LEFT ? "LEFT" : "RIGHT") << ") " << lowestCounts[2] << std::endl;
    
    return bestSplitPlane;
}

void KdTree::SHACost(float* outCost, SHASplitPlane::Side* outSide, float plane, unsigned aaAxis, const AABBox& voxel, unsigned numLeftPrims, unsigned numRightPrims, unsigned numPlanarPrims) const
{
    AABBox leftVoxel, rightVoxel;
    voxel.split(&leftVoxel, &rightVoxel, plane, aaAxis);
    
    float probabilityHitLeft = leftVoxel.surfaceArea() / voxel.surfaceArea();
    float probabilityHitRight = rightVoxel.surfaceArea() / voxel.surfaceArea();
    
    float costLeft = calculateSplitCost(probabilityHitLeft, probabilityHitRight, numLeftPrims + numPlanarPrims, numRightPrims);
    float costRight = calculateSplitCost(probabilityHitLeft, probabilityHitRight, numLeftPrims, numRightPrims + numPlanarPrims);
    
    if (costLeft < costRight)
    {
        *outCost = costLeft;
        *outSide = SHASplitPlane::LEFT;
    }
    else
    {
        *outCost = costRight;
        *outSide = SHASplitPlane::RIGHT;
    }
}

void KdTree::DumpSplitEvents(SHAPlaneEventList::const_iterator begin, SHAPlaneEventList::const_iterator end, unsigned aaAxis) const
{
    unsigned axis = std::numeric_limits<unsigned>::max();
    for (; begin != end; ++begin)
    {
        if (aaAxis < 3 && begin->axis != aaAxis)
        {
            continue;
        }

        if (begin->axis != axis)
        {
            axis = begin->axis;
            std::cout << std::endl << "---------- Axis " << axis << "----------" << std::endl;
        }
        
        std::cout << "\t" << "(" << begin->primitive->id() << ") Plane: " << begin->plane;
        
        glm::vec3 ll, ur;
        begin->primitive->bounds(ll, ur);
        std::cout << " Bounds: " << ll[axis] << "," << ur[axis] << " Type: ";
        
        switch (begin->type) {
            case START:
                std::cout << "START";
                break;
            case END:
                std::cout << "END";
                break;
            case PLANAR:
                std::cout << "PLANAR";
                break;
                
            default:
                TP_ASSERT(0);
                break;
        }
        
        std::cout << std::endl;
    }
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

KdTree::SHAPlaneEvent::SHAPlaneEvent(IPrimitive* _prim, float _plane, unsigned _axis, SHAPlaneEventType _type)
    : primitive(_prim)
    , plane(_plane)
    , axis(_axis)
    , type(_type)
{
}

bool KdTree::SHAPlaneEvent::operator<(const SHAPlaneEvent& rhs) const
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
