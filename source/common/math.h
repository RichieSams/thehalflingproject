/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef MATH_H
#define MATH_H

#include "common/halfling_sys.h"


namespace Common {

template<typename T>
inline T Lerp(const T &a, const T &b, float t) {
	return a + (b - a) * t;
}

template<typename T>
inline T Clamp(const T &x, const T &low, const T &high) {
	return x < low ? low : (x > high ? high : x);
}

// Returns random float in [0, 1).
static float RandF() {
	return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
static float RandF(float a, float b) {
	return a + RandF() * (b - a);
}

} // End of namespace Common

#endif // MATHHELPER_H