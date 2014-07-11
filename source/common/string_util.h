/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include <string>


namespace Common {

const std::string kWhiteSpaces(" \f\n\r\t\v");
const std::wstring kWideWhiteSpaces(L" \f\n\r\t\v");

inline void RTrim(std::string &str, const std::string &trimChars = kWhiteSpaces) {
	str.erase(str.find_last_not_of(trimChars) + 1);
}

inline void LTrim(std::string &str, const std::string &trimChars = kWhiteSpaces) {
	str.erase(0, str.find_first_not_of(trimChars));
}

inline void Trim(std::string &str, const std::string &trimChars = kWhiteSpaces) {
	RTrim(str, trimChars);
	LTrim(str, trimChars);
}

template <class ContainerT>
void Tokenize(const std::string& str, ContainerT& tokens, const std::string& delimiters = kWhiteSpaces, bool trimEmpty = false) {
	typedef typename ContainerT::value_type ValueType; 
	typedef typename ValueType::size_type SizeType;
	
	std::string::size_type pos, lastPos = 0;
	while (true) {
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == std::string::npos) {
			pos = str.length();

			if (pos != lastPos || !trimEmpty) {
				tokens.push_back(ValueType(str.data() + lastPos, (SizeType)pos - lastPos));
			}

			break;
		} else {
			if (pos != lastPos || !trimEmpty) {
				tokens.push_back(ValueType(str.data() + lastPos, (SizeType)pos - lastPos));
			}
		}

		lastPos = pos + 1;
	}
};

template <class ContainerT>
void Tokenize(const std::wstring& str, ContainerT& tokens, const std::wstring& delimiters = kWideWhiteSpaces, bool trimEmpty = false) {
	typedef typename ContainerT::value_type ValueType;
	typedef typename ValueType::size_type SizeType;

	std::wstring::size_type pos, lastPos = 0;
	while (true) {
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == std::wstring::npos) {
			pos = str.length();

			if (pos != lastPos || !trimEmpty) {
				tokens.push_back(ValueType(str.data() + lastPos, (SizeType)pos - lastPos));
			}

			break;
		} else {
			if (pos != lastPos || !trimEmpty) {
				tokens.push_back(ValueType(str.data() + lastPos, (SizeType)pos - lastPos));
			}
		}

		lastPos = pos + 1;
	}
};

bool Replace(std::string &str, const std::string &from, const std::string &to);
std::string &ReplaceAll(std::string &str, const std::string &from, const std::string &to);

} // End of namespace Common
