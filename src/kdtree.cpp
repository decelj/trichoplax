#include <sys/ioctl.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <iterator>
#include <unordered_map>

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

void dumpPrimitiveListBounds(std::vector<Triangle*, AlignedAllocator<Triangle*> > prims, unsigned axis)
{
    std::sort(prims.begin(), prims.end(), [axis](Triangle* a, Triangle* b)
              { glm::vec3 all, aur, bll, bur; a->bounds(all, aur); b->bounds(bll, bur); return all[axis] < bll[axis]; });

    glm::vec3 ll, ur;
    for (auto it  = prims.begin(); it != prims.end(); ++it)
    {
        (*it)->bounds(ll, ur);
        std::cout << "(" << (*it)->id() << ") " << ll[axis] << ", " << ur[axis] << std::endl;
    }
}
    
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

const uint32_t KdTree::Node::sInvalidNodeIdx = 0xFFFFFFFF;

KdTree::KdTree()
    : mNodes(2)
    , mMaxDepth(0)
    , mMinDepth(std::numeric_limits<unsigned>::max())
    , mLargeNodes(0)
    , mLeafNodes(0)
    , mTotalNodes(0)
    , mMaxPrimsPerNode(0)
    , mTotalNumPrims(0)
{
}

KdTree::~KdTree()
{
}

KdTree::TraversalBuffer KdTree::allocateTraversalBuffer() const
{
    return TraversalBuffer(mMaxDepth + 2, Node::sInvalidNodeIdx);
}

void KdTree::build()
{
    HighResTimer t;
    t.start();
    
    root().updateBounds();
    mTotalNumPrims = root().primitiveCount();
    
    SHAPlaneEventList initialEvents;
    Node::ConstPrimIterator it = root().beginPrimitives();
    for (; it != root().endPrimitives(); ++it)
    {
        generateEventsForPrimitive(*it, root().bounds, initialEvents);
    }
    
    initialEvents.sort();
    uint32_t nextNodeIdx = 1; // idx 0 is root node
    build(NodePtr(0, mNodes), initialEvents, 0, &nextNodeIdx);
    mNodes.shrink_to_fit();

    std::cout << "KdTree Build Stats:" << std::endl;
    std::cout << std::left << std::setw(30) << "  Max depth:" << mMaxDepth << std::endl;
    std::cout << std::left << std::setw(30) << "  Min depth:" << mMinDepth << std::endl;
    std::cout << std::left << std::setw(30) << "  Max primitives per node:" << mMaxPrimsPerNode << std::endl;
    std::cout << std::left << std::setw(30) << "  Total nodes:" << mTotalNodes << std::endl;
    std::cout << std::left << std::setw(30) << "  Leaf nodes:" << mLeafNodes << std::endl;
    std::cout << std::left << std::setw(30) << "  Total primitives:" << mTotalNumPrims << std::endl;
    std::cout << std::left << std::setw(30) << "  Build time:" << t.elapsedToString(t.elapsed()) << std::endl;
}

void KdTree::build(NodePtr node, SHAPlaneEventList& events, unsigned int depth, uint32_t* nextNodeIdx)
{
    ++mTotalNodes;

    const float leafCost = INTERSECTION_COST * node->primitiveCount();
    float splitCost = 0.f;
    SHASplitPlane splitPlane = findSplitPlane(&splitCost, node->bounds, events, node->primitiveCount());

    // Base case
    if (leafCost < splitCost || node->primitiveCount() == 1)
    {
        mMaxDepth = std::max(depth, mMaxDepth);
        mMinDepth = std::min(depth, mMinDepth);
        mMaxPrimsPerNode = std::max(node->primitiveCount(), mMaxPrimsPerNode);
        ++mLeafNodes;
    }
    else
    {
        SHAPlaneEventList leftEvents, rightEvents;
        split(leftEvents, rightEvents, node, splitPlane, events, nextNodeIdx);
        events.clear();

        NodePtr leftChild = NodePtr(node->leftIdx(), mNodes);
        NodePtr rightChild = NodePtr(node->rightIdx(), mNodes);

        build(leftChild, leftEvents, depth+1, nextNodeIdx);
        build(rightChild, rightEvents, depth+1, nextNodeIdx);

        if (leftChild && leftChild->isLeaf() && leftChild->isEmpty())
        {
            TP_ASSERT(rightChild && (!rightChild->isEmpty() || !rightChild->isLeaf()));
            node->setChildren(Node::sInvalidNodeIdx, node->rightIdx());
            --mLeafNodes;
            --mTotalNodes;
        }

        if (rightChild && rightChild->isLeaf() && rightChild->isEmpty())
        {
            TP_ASSERT(leftChild && (!leftChild->isEmpty() || !leftChild->isLeaf()));
            node->setChildren(node->leftIdx(), Node::sInvalidNodeIdx);
            --mLeafNodes;
            --mTotalNodes;
        }
    }
}

void KdTree::split(SHAPlaneEventList& outLeftEvents, SHAPlaneEventList& outRightEvents,
                   NodePtr node, const SHASplitPlane& plane, SHAPlaneEventList& events,
                   uint32_t* nextNodeIdx)
{
    enum PrimSide
    
    {
        PS_LEFT = 0,
        PS_RIGHT,
        PS_BOTH
    };
    
    typedef std::unordered_map<const Triangle*, PrimSide> PrimSideMap;
    
    /* Classify the primitives based on the split plane and the split events */
    PrimSideMap primClassification(node->primitiveCount() * 2);
    for (Node::ConstPrimIterator it = node->beginPrimitives(); it != node->endPrimitives(); ++it)
    {
        primClassification[*it] = PS_BOTH;
    }

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
    }

    *nextNodeIdx += 2;
    if ((*nextNodeIdx) >= mNodes.size())
    {
        mNodes.resize(mNodes.size() << 1);
    }

    /* Split the node */
    node->clearPrimitves();
    node->setChildren(*nextNodeIdx - 2, *nextNodeIdx - 1);
    node->bounds.split(&mNodes[node->leftIdx()].bounds,
                       &mNodes[node->rightIdx()].bounds,
                       plane.aaAxis, plane.plane);

    NodePtr leftChild = NodePtr(node->leftIdx(), mNodes);
    NodePtr rightChild = NodePtr(node->rightIdx(), mNodes);
    for (PrimSideMap::const_iterator it = primClassification.begin(); it != primClassification.end(); ++it)
    {
        switch (it->second)
        {
            case PS_LEFT:
                leftChild->addPrimitive(it->first);
                break;
                
            case PS_RIGHT:
                rightChild->addPrimitive(it->first);
                break;
                
            case PS_BOTH:
            default:
                leftChild->addPrimitive(it->first);
                rightChild->addPrimitive(it->first);
                break;
        }
    }
    
    /* Generate SHAPlaneEvents for left and right nodes */
    SHAPlaneEventList newLeftEvents;
    SHAPlaneEventList newRightEvents;

    for (SHAPlaneEventList::const_iterator it = events.begin(); it != events.end(); ++it)
    {
        PrimSideMap::const_iterator primIt = primClassification.find(it->primitive);
        if (primIt == primClassification.end())
        {
            continue;
        }

        switch (primIt->second)
        {
        case PS_LEFT:
            TP_ASSERT(
                std::find(rightChild->beginPrimitives(),
                          rightChild->endPrimitives(), primIt->first)
                    == rightChild->endPrimitives());
            outLeftEvents.emplace_back(*it);
            break;
            
        case PS_RIGHT:
            TP_ASSERT(
                std::find(leftChild->beginPrimitives(),
                          leftChild->endPrimitives(), primIt->first)
                    == leftChild->endPrimitives());
            outRightEvents.emplace_back(*it);
            break;
            
        case PS_BOTH:
            TP_ASSERT(
                  std::find(leftChild->beginPrimitives(),
                            leftChild->endPrimitives(), primIt->first)
                  != leftChild->endPrimitives());
            TP_ASSERT(
                  std::find(rightChild->beginPrimitives(),
                            rightChild->endPrimitives(), primIt->first)
                  != rightChild->endPrimitives());

            generateEventsForPrimitive(it->primitive,
                                       leftChild->bounds, newLeftEvents);
            generateEventsForPrimitive(it->primitive,
                                       rightChild->bounds, newRightEvents);
            primClassification.erase(primIt);
            break;

        default:
            break;
        }
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

bool KdTree::trace(Ray& ray, bool firstHit, TraversalBuffer& traversalStack, Mailboxer& mailboxes, Stats& threadStats) const
{
    threadStats.boxTests++;
    if (!mNodes[0].intersectBounds(ray))
        return false;

    //const glm::bvec3 frontNodeIsPositive = glm::lessThan(glm::vec3(0.f), ray.dir());
    bool hitPrimitive = false;
    uint32_t lastChild = 0;
    traversalStack[1] = 0;
    TraversalBuffer::iterator currentStackIt = traversalStack.begin() + 1;

    while (*currentStackIt != Node::sInvalidNodeIdx)
    {
        uint32_t currentNodeIdx = *currentStackIt;
        const Node& currentNode = mNodes[currentNodeIdx];
        if (currentNode.isLeaf())
        {
            Node::ConstPrimIterator it = currentNode.beginPrimitives();
            for (; it != currentNode.endPrimitives() && !(firstHit && hitPrimitive); ++it)
            {
                if (!mailboxes.Tested((*it)->id()))
                {
                    threadStats.primitiveTests++;
                    if ((*it)->intersect(ray))
                    {
                        if (firstHit)
                        {
                            return true;
                        }

                        hitPrimitive = true;
                    }

                    mailboxes.Mark((*it)->id());
                }
            }

            // Pop leaf node
            TP_ASSERT(currentStackIt != traversalStack.begin());
            --currentStackIt;
            lastChild = currentNodeIdx;
        }
        else if (lastChild == currentNode.rightIdx())
        {
            // Finished traversing all descendants of this node, pop
            TP_ASSERT(currentStackIt != traversalStack.begin());
            --currentStackIt;
            lastChild = currentNodeIdx;
        }
        else if (lastChild == currentNode.leftIdx())
        {
            bool haveRightChild = currentNode.rightIdx() != Node::sInvalidNodeIdx;
            if (haveRightChild && mNodes[currentNode.rightIdx()].intersectBounds(ray))
            {
                // Hit right node, push right node
                ++currentStackIt;
                TP_ASSERT(currentStackIt != traversalStack.end());
                *currentStackIt = currentNode.rightIdx();
            }
            else
            {
                // Finished traversing all descendants of this node, pop
                TP_ASSERT(currentStackIt != traversalStack.begin());
                --currentStackIt;
                lastChild = currentNodeIdx;
            }

            if (haveRightChild)
            {
                threadStats.boxTests++;
            }
        }
        else
        {
            bool haveLeftChild = currentNode.leftIdx() != Node::sInvalidNodeIdx;
            bool haveRightChild = currentNode.rightIdx() != Node::sInvalidNodeIdx;
            if (haveLeftChild && mNodes[currentNode.leftIdx()].intersectBounds(ray))
            {
                // Hit left node, push left node
                ++currentStackIt;
                TP_ASSERT(currentStackIt != traversalStack.end());
                *currentStackIt = currentNode.leftIdx();
            }
            else
            {
                if (haveRightChild &&
                   (haveLeftChild || mNodes[currentNode.rightIdx()].intersectBounds(ray)))
                {
                    // If we didn't hit the left node we better hit the right one
                    // otherwise why did the parent node box test succeed?
                    if (haveLeftChild)
                    {
                        TP_ASSERT(mNodes[currentNode.rightIdx()].intersectBounds(ray));
                    }

                    // Hit right node only, push right node
                    ++currentStackIt;
                    TP_ASSERT(currentStackIt != traversalStack.end());
                    *currentStackIt = currentNode.rightIdx();
                }
                else
                {
                    // Finished traversing all descendants of this node, pop
                    TP_ASSERT(currentStackIt != traversalStack.begin());
                    --currentStackIt;
                    lastChild = currentNodeIdx;
                }
            }

            threadStats.boxTests++;
        }
    }
    
    return hitPrimitive;
}

void KdTree::generateEventsForPrimitive(const Triangle* primitive, const AABBox& voxel, SHAPlaneEventList& events) const
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
    
    return bestSplitPlane;
}

void KdTree::SHACost(float* outCost, SHASplitPlane::Side* outSide, float plane, unsigned aaAxis, const AABBox& voxel, unsigned numLeftPrims, unsigned numRightPrims, unsigned numPlanarPrims) const
{
    AABBox leftVoxel, rightVoxel;
    voxel.split(&leftVoxel, &rightVoxel, aaAxis, plane);
    
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


KdTree::Node::Node()
    : bounds()
    , mLeft(sInvalidNodeIdx)
    , mRight(sInvalidNodeIdx)
    , mPrims()
{
}

KdTree::Node::~Node()
{
    mPrims.clear();
}

void KdTree::Node::updateBounds()
{
    glm::vec3 ur(std::numeric_limits<float>::lowest());
    glm::vec3 ll(std::numeric_limits<float>::max());
    glm::vec3 primUr;
    glm::vec3 primLl;
    
    ConstPrimIterator it = beginPrimitives();
    for (; it != endPrimitives(); ++it)
    {
        (*it)->bounds(primLl, primUr);

        for (int i = 0; i < 3; i++)
        {
            ur[i] = std::max(ur[i], primUr[i]);
            ll[i] = std::min(ll[i], primLl[i]);
        }
    }
    
    bounds.update(ll, ur);
}

KdTree::SHAPlaneEvent::SHAPlaneEvent(const Triangle* _prim, float _plane, unsigned _axis, SHAPlaneEventType _type)
    : primitive(_prim)
    , plane(_plane)
    , axis(_axis)
    , type(_type)
{
}
