/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_STRING_UTIL_H
#define COMMON_STRING_UTIL_H

#include <string>


namespace Common {

const std::string kWhiteSpaces(" \f\n\r\t\v");

inline void RTrim(std::string& str, const std::string& trimChars = kWhiteSpaces) {
	str.erase(str.find_last_not_of(trimChars) + 1);
}

inline void LTrim(std::string& str, const std::string& trimChars = kWhiteSpaces) {
	str.erase(0, str.find_first_not_of(trimChars));
}

inline void Trim(std::string& str, const std::string& trimChars = kWhiteSpaces) {
	RTrim(str, trimChars);
	LTrim(str, trimChars);
}

} // End of namespace Common

#endif