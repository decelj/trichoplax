//
//  mailboxer.h
//  trichoplax
//
//  Created by Justin DeCell on 6/15/15.
//  Copyright (c) 2015 com.jdecell. All rights reserved.
//

#ifndef __trichoplax__mailboxer__
#define __trichoplax__mailboxer__

#include <vector>
#include "alligned_allocator.h"

class Mailboxer
{
public:
    explicit Mailboxer(size_t count);
    ~Mailboxer();
    
    inline void IncrementRayId()
    { ++mCurrentRayId; }
    
    inline bool Tested(const size_t& primId)
    { return mMailboxes[primId] == mCurrentRayId; }
    
    inline void Mark(const size_t& primId)
    { mMailboxes[primId] = mCurrentRayId; }
    
private:
    static AllignedAllocator<unsigned>  s_Allocator;
    
    unsigned    mCurrentRayId;
    std::vector<unsigned, AllignedAllocator<unsigned> >     mMailboxes;
    
    /* Not copyable */
    Mailboxer(const Mailboxer&);
    Mailboxer& operator=(const Mailboxer&);
};


#endif /* defined(__trichoplax__mailboxer__) */
