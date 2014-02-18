/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_HASH_H
#define COMMON_HASH_H

#include <tuple>
#include <functional>
#include <type_traits>

inline size_t hash_combiner(size_t left, size_t right) {
	// FNV hash
	return ((2166136261 * 16777619) ^ left) * 16777619 ^ right;
}

namespace std {

template<typename... TTypes>
class hash<std::tuple<TTypes...>> {
private:
	typedef std::tuple<TTypes...> Tuple;

	template<int N>
	size_t operator()(Tuple value) const { return 0; }

	template<int N, typename THead, typename... TTail>
	size_t operator()(Tuple value) const {
		const int Index = N - sizeof...(TTail)-1;
		return hash_combiner(hash<THead>()(std::get<Index>(value)), operator()<N, TTail...>(value));
	}

public:
	size_t operator()(Tuple value) const {
		return operator()<sizeof...(TTypes), TTypes...>(value);
	}
};

} // End of namespace std

#endif
