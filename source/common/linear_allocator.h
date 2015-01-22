/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"


namespace Common {

class LinearAllocator {
public:
    LinearAllocator(size_t pageSize);
	~LinearAllocator();

private:
    struct Page {
        Page(size_t pageSize)
            : NextPage(nullptr),
              Data(::operator new(pageSize)) {
        }
    
        Page *NextPage;
        void *Data;
    };

    size_t m_pageSize;
    
    Page *m_firstPage;
    Page *m_currentPage;
    
    byte *m_start;
    byte *m_end;
    byte *m_current;
    
public:
    void *Allocate(size_t size);
    inline void Reset() { m_current = m_start; }
};

} // End of namespace Common
