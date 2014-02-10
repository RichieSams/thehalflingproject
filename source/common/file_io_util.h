/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_FILE_IO_UTIL_H
#define COMMON_FILE_IO_UTIL_H

#include <fstream>


namespace Common {

/**
 * Reads a line from an istream using /r, /r/n, and /n as line delimiters
 * 
 * @param inputStream    The stream to read from
 * @param output         Will be filled with the line read
 * @return               The input stream
 */
std::istream& SafeGetLine(std::istream& inputStream, std::string& output) {
	output.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(inputStream, true);
	std::streambuf* sb = inputStream.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return inputStream;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return inputStream;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (output.empty())
				inputStream.setstate(std::ios::eofbit);
			return inputStream;
		default:
			output += (char)c;
		}
	}
}

} // End of namespace Common

#endif
