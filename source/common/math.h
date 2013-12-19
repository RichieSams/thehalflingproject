/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef MATH_H
#define MATH_H


namespace Common {

template<typename T>
inline T Min(const T& a, const T& b) {
	return a < b ? a : b;
}

template<typename T>
inline T Max(const T& a, const T& b) {
	return a > b ? a : b;
}

template<typename T>
inline T Lerp(const T& a, const T& b, float t) {
	return a + (b - a)*t;
}

template<typename T>
inline T Clamp(const T& x, const T& low, const T& high) {
	return x < low ? low : (x > high ? high : x);
}

} // End of namespace Common

#endif // MATHHELPER_H