/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

/*
 * This code is pretty much a copy of the example code presented
 * in this Microsoft blog post: 
 * http://blogs.msdn.com/b/vcblog/archive/2008/08/28/the-mallocator.aspx
 *
 * The only differences are in allocate()
 */


#ifndef COMMON_ALLOCATOR_ALIGNED16_H
#define COMMON_ALLOCATOR_ALIGNED16_H

#include <stddef.h>  // Required for size_t and ptrdiff_t and NULL
#include <new>       // Required for placement new and std::bad_alloc
#include <stdexcept> // Required for std::length_error

#include <stdlib.h>  // For malloc() and free()
#include <iostream>  // For std::cout
#include <ostream>   // For std::endl


namespace Common {

template <typename T> class Allocator16Aligned {
public:
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	T *address(T& r) const {
		return &r;
	}

	const T *address(const T& s) const {
		return &s;
	}

	size_t max_size() const {
		// The following has been carefully written to be independent of
		// the definition of size_t and to avoid signed/unsigned warnings.
		return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
	}


	// The following must be the same for all allocators.
	template <typename U> struct rebind {
		typedef Allocator16Aligned<U> other;
	};

	bool operator!=(const Allocator16Aligned &other) const {
		return !(*this == other);
	}

	void construct(T *const p, const T &t) const {
		void * const pv = static_cast<void *>(p);

		new (pv)T(t);
	}

	void destroy(T * const p) const {
		p->~T();
	}


	// Returns true if and only if storage allocated from *this
	// can be deallocated from other, and vice versa.
	// Always returns true for stateless allocators.
	bool operator==(const Allocator16Aligned& other) const {
		return true;
	}

	// Default constructor, copy constructor, rebinding constructor, and destructor.
	// Empty for stateless allocators.
	Allocator16Aligned() {}
	Allocator16Aligned(const Allocator16Aligned &) {}
	template <typename U> Allocator16Aligned(const Allocator16Aligned<U> &) {}
	~Allocator16Aligned() {}

	// The following will be different for each allocator.
	T *allocate(const size_t n) const {
		// The return value of allocate(0) is unspecified.
		// Allocator16Aligned returns NULL in order to avoid depending
		// on malloc(0)'s implementation-defined behavior
		// (the implementation can define malloc(0) to return NULL,
		// in which case the bad_alloc check below would fire).
		// All allocators can return NULL in this case.
		if (n == 0) {
			return NULL;
		}

		// All allocators should contain an integer overflow check.
		// The Standardization Committee recommends that std::length_error
		// be thrown in the case of integer overflow.
		if (n > max_size()) {
			throw std::length_error("Mallocator<T>::allocate() - Integer overflow.");
		}

		void * const pv = _aligned_malloc(n * sizeof(T), 16);

		// Allocators should throw std::bad_alloc in the case of memory allocation failure.
		if (pv == NULL) {
			throw std::bad_alloc();
		}

		return static_cast<T *>(pv);
	}

	void deallocate(T * const p, const size_t n) const {
		_aligned_free(p);
	}


	// The following will be the same for all allocators that ignore hints.
	template <typename U> T *allocate(const size_t n, const U * /* const hint */) const {
		return allocate(n);
	}

private:
	// Allocators are not required to be assignable, so
	// all allocators should have a private unimplemented
	// assignment operator. Note that this will trigger the
	// off-by-default (enabled under /Wall) warning C4626
	// "assignment operator could not be generated because a
	// base class assignment operator is inaccessible" within
	// the STL headers, but that warning is useless.
	Allocator16Aligned& operator=(const Allocator16Aligned&);
};

} // End of namespace Common

#endif
