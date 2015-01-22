/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/linear_allocator.h"


namespace Common {

LinearAllocator::LinearAllocator(size_t pageSize)
	: m_pageSize(pageSize) {
	  m_currentPage = m_firstPage = new Page(pageSize);
}

LinearAllocator::~LinearAllocator() {
	Page *currentPage = m_firstPage;
	Page *pageToDelete;

	do {
		pageToDelete = currentPage;
		currentPage = currentPage->NextPage;

		delete pageToDelete;
	} while (currentPage != nullptr);
}

void *LinearAllocator::Allocate(size_t size) {
    if (m_current + size >= m_end) {
        // Allocate a new page;
        Page *newPage = new Page(m_pageSize);
        
        m_currentPage->NextPage = newPage;
        m_current = m_start = reinterpret_cast<byte *>(newPage->Data);
        m_end = m_start + m_pageSize;
    }
    
    void* userPtr = m_current;
    m_current += size;
    
    return userPtr;
}

} // End of namespace Common
