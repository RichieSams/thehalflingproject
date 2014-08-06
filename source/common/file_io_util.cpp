/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/file_io_util.h"


namespace Common {

char *ReadWholeFile(const wchar *name, DWORD *bytesRead) {
	HANDLE hFile = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL; // error condition, could call GetLastError to find out more

	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size)) {
		CloseHandle(hFile);
		return NULL; // error condition, could call GetLastError to find out more
	}

	DWORD fileSize = (DWORD)size.QuadPart;
	char *fileBuffer = new char[fileSize];
	if (!ReadFile(hFile, fileBuffer, fileSize, bytesRead, NULL)) {
		return nullptr;
	}

	CloseHandle(hFile);

	return fileBuffer;
}
	
std::istream &SafeGetLine(std::istream &inputStream, std::string &output) {
	output.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(inputStream, true);
	std::streambuf *sb = inputStream.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return inputStream;
		case '\r':
			if (sb->sgetc() == '\n') {
				sb->sbumpc();
			}
			return inputStream;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (output.empty()) {
				inputStream.setstate(std::ios::eofbit);
			}
			return inputStream;
		default:
			output += (char)c;
		}
	}
}

std::ostream &BinaryWriteUInt64(std::ostream &stream, const uint64 &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(uint64));
}
std::ostream &BinaryWriteInt64(std::ostream &stream, const int64 &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(int64));
}

std::ostream &BinaryWriteUInt32(std::ostream &stream, const uint32 &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(uint32));
}
std::ostream &BinaryWriteInt32(std::ostream &stream, const int32 &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(int32));
}

std::ostream &BinaryWriteUInt16(std::ostream &stream, const uint16 &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(uint16));
}
std::ostream &BinaryWriteInt16(std::ostream &stream, const int16 &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(int16));
}

std::ostream &BinaryWriteByte(std::ostream &stream, const byte &value) {
	return stream.write(reinterpret_cast<const char *>(&value), sizeof(byte));
}

} // End of namespace Common
