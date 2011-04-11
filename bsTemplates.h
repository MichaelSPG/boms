#ifndef BS_TEMPLATES_H
#define BS_TEMPLATES_H

#include <vector>
#include <map>

namespace bsT
{

/*	Erases an element from anywhere in the vector in constant time, but the order of the
	remaining elements will be undefined.

	Note that it is possible to abuse this function like this:
	std::vector<std::string> animals;
	animals.push_back("dog"); animals.push_back("cat"); animals.push_back("horse");
	std::string unrelated("aeroplane");
	unordered_erase(animals, unrelated);//unrelated now contains "horse" and an incorrect
										//element has been removed from the vector

	The correct way to use this function is like this:
	unordered_erase(animals, animals[1]);
*/
template <typename T>
inline void unordered_erase(std::vector<T>& vec, T& elem)
{
	elem = vec.back();
	vec.pop_back();
}

/*	Removes every element from the map for which pred evaluates to true.
	TODO: See if this actually works
*/
template<typename Key, typename Value, typename Comp, typename Alloc, typename Predicate>
void map_remove_if(std::map<Key, Value, typename Comp, typename Alloc>& m, const Predicate& pred)
{
	for (std::map<Key, Value, typename Comp, typename Alloc>::iterator itr = m.begin(),
		end = m.end(); itr != end; /*nothing*/)
	{
		if (pred(*itr))
		{
			std::map<Key, Value>::iterator eraseMe = itr;
			//Don't invalidate the iterator
			++itr;
			m.erase(eraseMe);
		}
		else
		{
			++itr;
		}
	}
}

} // bsT

#endif // BS_TEMPLATES_H
