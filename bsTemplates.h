#ifndef BS_TEMPLATES_H
#define BS_TEMPLATES_H

#include <vector>


/**	Erases an element from a vector, but does not sort it afterwards.
*/
template <typename T>
inline void unordered_erase(std::vector<T>& vec, T& element)
{
	std::swap(element, vec.back());
	vec.pop_back();
}

#endif // BS_TEMPLATES_H
