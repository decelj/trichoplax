//
//  mailboxer.cpp
//  trichoplax
//
//  Created by Justin DeCell on 6/15/15.
//  Copyright (c) 2015 com.jdecell. All rights reserved.
//

#include "mailboxer.h"

AllignedAllocator<unsigned> Mailboxer::s_Allocator = AllignedAllocator<unsigned>(64);

Mailboxer::Mailboxer(size_t count) :
    mCurrentRayId(0),
    mMailboxes(count, 0, s_Allocator)
{}

Mailboxer::~Mailboxer()
{
    mMailboxes.clear();
}
