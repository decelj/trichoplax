#include <sys/ioctl.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>

#include "kdtree.h"
#include "aabbox.h"
#include "common.h"
#include "triangle.h"
#include "ray.h"
#include "timer.h"
#include "mailboxer.h"
#include "stats.h"

#define TRAVERSAL_COST (15.f)
#define INTERSECTION_COST (20.f)

namespace
{
inline float calculateSplitCost(float probabilityHitLeft, float probabilityHitRight, unsigned numPrimsLeft, unsigned numPrimsRight)
{
    // TODO: This causes infinite recursion trying to create empty nodes with zero volume. Fix me.
    //float modifer = numPrimsLeft == 0 || numPrimsRight == 0 ? .8f : 1.f;
    float cost = INTERSECTION_COST * (probabilityHitLeft * (float)numPrimsLeft + probabilityHitRight * (float)numPrimsRight);
    cost += TRAVERSAL_COST;
    //cost *= modifer;

    return cost;
}
} // anonymous namespace

KdTree::KdTree()
    : mNodes(2)
    , mMaxDepth(0)
    , mMinDepth(std::numeric_limits<unsigned>::max())
    , mLeafNodes(0)
    , mTotalNodes(0)
    , mMaxPrimsPerNode(0)
{
}

KdTree::~KdTree()
{
}

KdTree::TraversalBuffer KdTree::allocateTraversalBuffer() const
{
    return TraversalBuffer(mMaxDepth);
}

void KdTree::build()
{
    HighResTimer t;
    t.start();

    mBounds = mPrimVector[0]->bounds();
    for (const Triangle* tri : mPrimVector)
    {
        mBounds = mBounds.join(tri->bounds());
    }
    
    SHAPlaneEventList initialEvents;
    for (const Triangle* tri : mPrimVector)
    {
        generateEventsForPrimitive(tri, mBounds, initialEvents);
    }
    initialEvents.sort();

    uint32_t nextNodeIdx = 1; // idx 0 is root node
    build(0, mBounds, initialEvents, mPrimVector.size(), 0, &nextNodeIdx);
    mNodes.shrink_to_fit();

    std::cout << "KdTree Build Stats:" << std::endl;
    std::cout << std::left << std::setw(30) << "  Max depth:" << mMaxDepth << std::endl;
    std::cout << std::left << std::setw(30) << "  Min depth:" << mMinDepth << std::endl;
    std::cout << std::left << std::setw(30) << "  Max primitives per node:" << mMaxPrimsPerNode << std::endl;
    std::cout << std::left << std::setw(30) << "  Total nodes:" << mTotalNodes << std::endl;
    std::cout << std::left << std::setw(30) << "  Leaf nodes:" << mLeafNodes << std::endl;
    std::cout << std::left << std::setw(30) << "  Total primitives:" << mPrimVector.size() << std::endl;
    std::cout << std::left << std::setw(30) << "  Build time:" << t.elapsedToString(t.elapsed()) << std::endl;
}

void KdTree::build(uint32_t nodeIdx, const AABBox& bounds, SHAPlaneEventList& events, unsigned numPrimitives, unsigned depth, uint32_t* nextNodeIdx)
{
    TP_ASSERT(events.size() <= numPrimitives * 2 * 3);
    TP_ASSERT(events.size() >= numPrimitives);

    const float leafCost = INTERSECTION_COST * numPrimitives;
    SHASplitPlane splitPlane = findSplitPlane(bounds, events, numPrimitives);

    // Base case
    if (leafCost < splitPlane.cost || numPrimitives <= 1)
    {
        using PrimSet = std::unordered_set<const Triangle*>;
        PrimSet uniquePrims;

        Node& node = mNodes[nodeIdx];
        node.initLeafNode(numPrimitives);
        Node::PrimIterator curr = node.beginPrimitives();

        for (SHAPlaneEvent event : events)
        {
            if (uniquePrims.insert(event.primitive).second)
            {
                TP_ASSERT(curr != node.endPrimitives());
                *curr++ = event.primitive;
            }
        }
        TP_ASSERT(curr == node.endPrimitives());

        mMaxDepth = std::max(depth, mMaxDepth);
        mMinDepth = std::min(depth, mMinDepth);
        mMaxPrimsPerNode = std::max(node.primitiveCount(), mMaxPrimsPerNode);
        ++mLeafNodes;
    }
    else
    {
        AABBox leftBounds, rightBounds;
        bounds.split(&leftBounds, &rightBounds, splitPlane.aaAxis, splitPlane.plane);

        SHAPlaneEventList leftEvents, rightEvents;
        split(leftEvents, rightEvents, leftBounds, rightBounds, splitPlane, events);
        events.clear();

        uint32_t leftChildIdx = allocNode(nextNodeIdx);
        build(leftChildIdx, leftBounds, leftEvents,
              splitPlane.numPrimitivesLeft, depth + 1, nextNodeIdx);

        uint32_t rightChildIdx = allocNode(nextNodeIdx);
        build(rightChildIdx, rightBounds, rightEvents,
              splitPlane.numPrimitivesRight, depth + 1, nextNodeIdx);

        Node& node = mNodes[nodeIdx];
        node.split(leftChildIdx, rightChildIdx, splitPlane.plane, splitPlane.aaAxis);
    }
}

void KdTree::split(SHAPlaneEventList& outLeftEvents, SHAPlaneEventList& outRightEvents,
                   const AABBox& leftBounds, const AABBox& rightBounds,
                   const SHASplitPlane& plane, SHAPlaneEventList& events)
{
    enum PrimSide
    {
        PS_LEFT = 0,
        PS_RIGHT,
        PS_BOTH
    };
    
    typedef std::unordered_map<const Triangle*, PrimSide> PrimSideMap;
    
    /* Classify the primitives based on the split plane and the split events */
    PrimSideMap primClassification;
    for (SHAPlaneEventList::const_iterator it = events.begin(); it != events.end(); ++it)
    {
        if (it->type == END && it->axis == plane.aaAxis && it->plane <= plane.plane)
        {
            primClassification[it->primitive] = PS_LEFT;
        }
        else if (it->type == START && it->axis == plane.aaAxis && it->plane >= plane.plane)
        {
            primClassification[it->primitive] = PS_RIGHT;
        }
        else if (it->type == PLANAR && it->axis == plane.aaAxis)
        {
            if (it->plane < plane.plane ||
                (it->plane == plane.plane && plane.side == SHASplitPlane::LEFT))
            {
                primClassification[it->primitive] = PS_LEFT;
            }
            else if (it->plane > plane.plane ||
                     (it->plane == plane.plane && plane.side == SHASplitPlane::RIGHT))
            {
                primClassification[it->primitive] = PS_RIGHT;
            }
        }
        else if (primClassification.find(it->primitive) == primClassification.end())
        {
            primClassification[it->primitive] = PS_BOTH;
        }
    }
    
    /* Generate SHAPlaneEvents for left and right nodes */
    std::unordered_set<const Triangle*> spanningPrimitives;
    for (const SHAPlaneEvent& event : events)
    {
        PrimSideMap::const_iterator primIt = primClassification.find(event.primitive);
        TP_ASSERT(primIt != primClassification.end());

        switch (primIt->second)
        {
        case PS_LEFT:
            outLeftEvents.emplace_back(event);
            break;
            
        case PS_RIGHT:
            outRightEvents.emplace_back(event);
            break;
            
        case PS_BOTH:
            spanningPrimitives.insert(event.primitive);
            break;

        default:
            break;
        }
    }

    SHAPlaneEventList newLeftEvents;
    SHAPlaneEventList newRightEvents;
    for (const Triangle* tri : spanningPrimitives)
    {
        generateEventsForPrimitive(tri, leftBounds, newLeftEvents);
        generateEventsForPrimitive(tri, rightBounds, newRightEvents);
    }
    newLeftEvents.sort();
    newRightEvents.sort();

    // Merge in left events
    SHAPlaneEventList::iterator outEventsIt = outLeftEvents.begin();
    SHAPlaneEventList::iterator newEventsIt = newLeftEvents.begin();
    for (; newEventsIt != newLeftEvents.end(); ++newEventsIt)
    {
        while (outEventsIt != outLeftEvents.end() && *outEventsIt < *newEventsIt)
        {
            ++outEventsIt;
        }
        
        outLeftEvents.insert(outEventsIt, *newEventsIt);
    }

    // Merge in right events
    outEventsIt = outRightEvents.begin();
    newEventsIt = newRightEvents.begin();
    for (; newEventsIt != newRightEvents.end(); ++newEventsIt)
    {
        while (outEventsIt != outRightEvents.end() && *outEventsIt < *newEventsIt)
        {
            ++outEventsIt;
        }
        
        outRightEvents.insert(outEventsIt, *newEventsIt);
    }
}

void KdTree::generateEventsForPrimitive(const Triangle* primitive, const AABBox& voxel, SHAPlaneEventList& events) const
{
    const AABBox primBox = primitive->bounds();
    const AABBox clippedBox = voxel.intersection(primBox);
    TP_ASSERT(clippedBox.isValid());
    
    // Clipped box width, height, depth
    const glm::vec3 clippedBoxWHD = clippedBox.ur() - clippedBox.ll();
    
    for (unsigned axis = 0; axis < 3; ++axis)
    {
        if (relEq(clippedBoxWHD[axis], 0.f, 1.0e-10f))
        {
            events.emplace_back(primitive, clippedBox.ll()[axis], axis, PLANAR);
        }
        else
        {
            // Sum of the areas of the voxel faces which are orthogonal to the
            // clip plane
            const float area = clippedBoxWHD[axis] * clippedBoxWHD[(axis + 1) % 3]
                + clippedBoxWHD[axis] * clippedBoxWHD[(axis + 2) % 3];
            TP_UNUSED(area);

            // Check for point/line intersection with voxel
            TP_ASSERT(!relEq(area,  0.f, 1.0e-15f));
            events.emplace_back(primitive, clippedBox.ll()[axis], axis, START);
            events.emplace_back(primitive, clippedBox.ur()[axis], axis, END);
        }
    }
}

KdTree::SHASplitPlane KdTree::findSplitPlane(
    const AABBox& voxel, const SHAPlaneEventList& events,
    unsigned totalNumPrimitives) const
{
    SHASplitPlane bestSplitPlane;
    unsigned numLeft[3] = {0, 0, 0};
    unsigned numPlanar[3] = {0, 0, 0};
    unsigned numRight[3] = {totalNumPrimitives, totalNumPrimitives, totalNumPrimitives};

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
        
        if (cost < bestSplitPlane.cost)
        {
            bestSplitPlane.plane = plane;
            bestSplitPlane.aaAxis = aaAxis;
            bestSplitPlane.side = side;
            bestSplitPlane.numPrimitivesRight = numRight[aaAxis] +
                (side == SHASplitPlane::RIGHT ? numPlanar[aaAxis] : 0);
            bestSplitPlane.numPrimitivesLeft = numLeft[aaAxis] +
                (side == SHASplitPlane::LEFT ? numPlanar[aaAxis] : 0);
            bestSplitPlane.cost = cost;

#if DEBUG
            lowestCounts[0] = numLeft[aaAxis];
            lowestCounts[1] = numRight[aaAxis];
            lowestCounts[2] = numPlanar[aaAxis];
#endif
        }
        
        numLeft[aaAxis] += startingAtPlane;
        numLeft[aaAxis] += planar;
    }
    
    return bestSplitPlane;
}

void KdTree::SHACost(float* outCost, SHASplitPlane::Side* outSide, float plane, unsigned aaAxis, const AABBox& voxel, unsigned numLeftPrims, unsigned numRightPrims, unsigned numPlanarPrims) const
{
    *outCost = std::numeric_limits<float>::max();
    *outSide = SHASplitPlane::RIGHT;

    if (numLeftPrims == 0 && numRightPrims == 0)
    {
        return;
    }

    AABBox leftVoxel, rightVoxel;
    voxel.split(&leftVoxel, &rightVoxel, aaAxis, plane);

    if (  numPlanarPrims == 0 &&
        ((leftVoxel.hasZeroVolume() && numLeftPrims == 0) ||
         (rightVoxel.hasZeroVolume() && numRightPrims == 0)))
    {
        return;
    }
    
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


KdTree::Node::Node()
    : mFlags(LEAF)
    , mPrimitives(nullptr)
{
}

KdTree::Node::Node(Node&& rhs)
    : mFlags(LEAF)
    , mPrimitives(nullptr)
{
    std::swap(mUpperChild, rhs.mUpperChild);
    std::swap(mPrimitives, rhs.mPrimitives);
}

KdTree::Node::~Node()
{
    if (isLeaf())
    {
        delete [] mPrimitives;
    }
}


KdTree::SHAPlaneEvent::SHAPlaneEvent(const Triangle* _prim, float _plane, unsigned _axis, SHAPlaneEventType _type)
    : primitive(_prim)
    , plane(_plane)
    , axis(_axis)
    , type(_type)
{
}


KdTree::SHASplitPlane::SHASplitPlane()
    : plane(std::numeric_limits<float>::max())
    , cost(std::numeric_limits<float>::max())
    , aaAxis(0)
    , side(LEFT)
    , numPrimitivesRight(0)
    , numPrimitivesLeft(0)
{
}
