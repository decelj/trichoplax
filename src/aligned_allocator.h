//
//  alligned_allocator.h
//  trichoplax
//
//  Created by Justin DeCell on 6/15/15.
//  Copyright (c) 2015 com.jdecell. All rights reserved.
//

#ifndef trichoplax_alligned_allocator_h
#define trichoplax_alligned_allocator_h

#include <limits>
#include <cstddef>
#include <stdlib.h>


template<typename T, size_t Alignment=64>
class AlignedAllocator
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_refernce;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    
    template<typename U>
    struct rebind
    {
        typedef AlignedAllocator<U> other;
    };
    
    AlignedAllocator() {}
    AlignedAllocator(const AlignedAllocator& other) {}
    template<typename U>
    AlignedAllocator(const AlignedAllocator<U>& other) {}
    
    ~AlignedAllocator() {}
    
    inline pointer allocate(size_type count, std::allocator<void>::const_pointer hint=0)
    {
        void* data;
        posix_memalign(&data, Alignment, count * sizeof(T));
        return reinterpret_cast<pointer>(data);
    }
    
    inline void deallocate(pointer p, size_type)
    {
        free(p);
    }
    
    inline bool operator==(const AlignedAllocator& other) const
    {
        return true;
    }
    inline bool operator!=(const AlignedAllocator& other) const
    {
        return false;
    }
};

#endif
