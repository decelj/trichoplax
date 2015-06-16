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
#include <assert.h>

template<typename T>
class AllignedAllocator
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
        typedef AllignedAllocator<U> other;
    };
    
    inline AllignedAllocator(const unsigned allignment) :
        m_Allignment(allignment)
    {
        assert(((m_Allignment - 1) & m_Allignment) == 0);
    }
    inline AllignedAllocator(const AllignedAllocator& other) :
        m_Allignment(other.m_Allignment)
    {}
    template<typename U>
    inline explicit AllignedAllocator(const AllignedAllocator<U>& other) :
        m_Allignment(other.allignment)
    {}
    
    inline ~AllignedAllocator()
    {}
    
    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_refernce r) { return &r; }
    
    inline pointer allocate(size_type count,
                            typename std::allocator<void>::const_pointer = 0)
    {
        void* data;
        posix_memalign(&data, m_Allignment, count * sizeof(T));
        return reinterpret_cast<pointer>(data);
    }
    inline void deallocate(pointer p, size_type)
    {
        free(p);
    }
    
    inline size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    
    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }
    
    inline bool operator==(const AllignedAllocator& other)
    {
        return other.m_Allignment == m_Allignment;
    }
    inline bool operator!=(const AllignedAllocator& other)
    {
        return !(other == *this);
    }
    
private:
    const unsigned m_Allignment;
};

#endif
