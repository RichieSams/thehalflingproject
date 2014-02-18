/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_MEMORY_STREAM_H
#define COMMON_MEMORY_STREAM_H

#include <istream>


namespace Common {

class MemoryInputStream : public std::istream {
public:
	MemoryInputStream(const char *aData, size_t aLength)
		: std::istream(&m_buffer),
		  m_buffer(aData, aLength) {
		rdbuf(&m_buffer); // reset the buffer after it has been properly constructed
	}

private:
	class MemoryBuffer : public std::basic_streambuf<char> {
	public:
		MemoryBuffer(const char *aData, size_t aLength) {
			setg((char *)aData, (char *)aData, (char *)aData + aLength);
		}
	};

	MemoryBuffer m_buffer;
};

} // End of namespace Common

#endif
