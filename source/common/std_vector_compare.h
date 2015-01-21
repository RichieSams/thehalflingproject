/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

// Modified from SO answer by JaredPar http://stackoverflow.com/questions/5225820/compare-two-vectors-c

#pragma once

#include <vector>


namespace Common {

template <typename T> 
bool CompareVectors(const std::vector<T> &lhs, const std::vector<T> &rhs) {
	auto leftIt = lhs.begin();
	auto rightIt = rhs.begin();

	if (lhs.size() != rhs.size()) {
		return false;
	}

	while (leftIt != lhs.end() && rightIt != rhs.end()) {
		if (*leftIt != *rightIt) {
			return false;
		}

		leftIt++;
		rightIt++;
	}

	return true;
}

} // End of namespace Common
