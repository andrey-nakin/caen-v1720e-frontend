#ifndef MATH_EdgeFinder_hxx
#define	MATH_EdgeFinder_hxx

#include <iterator>
#include <algorithm>
#include <functional>

namespace math {

template<class InputIt, class Comparator>
class EdgeFinder {

	enum class State {
		Init, Before, After
	};

public:

	typedef InputIt iterator_type;
	using value_type = typename std::iterator_traits<InputIt>::value_type;
	using reference = typename std::iterator_traits<InputIt>::reference;
	using difference_type = typename std::iterator_traits<InputIt>::difference_type;

	EdgeFinder(value_type const aThreshold) :
			threshold(aThreshold), comparator(), state(State::Init) {

	}

	bool operator()(reference const& v) {

		bool result = false;

		switch (state) {
		case State::Init:
			state = comparator(v, threshold) ? State::Before : State::After;
			break;

		case State::Before:
			if (!comparator(v, threshold)) {
				result = true;
			}
			break;

		case State::After:
			if (comparator(v, threshold)) {
				state = State::Before;
			}
			break;
		}

		return result;

	}

private:

	value_type const threshold;
	Comparator const comparator;
	State state;

};

template<class InputIt>
InputIt FindEdge(bool const rising,
		typename std::iterator_traits<InputIt>::value_type const threshold,
		InputIt const begin, InputIt const end) {

	return rising ?
			std::find_if(begin, end,
					EdgeFinder<InputIt,
							std::less<
									typename std::iterator_traits<InputIt>::value_type>>(
							threshold)) :
			std::find_if(begin, end,
					EdgeFinder<InputIt,
							std::greater<
									typename std::iterator_traits<InputIt>::value_type>>(
							threshold));

}

template<class InputIt>
typename std::iterator_traits<InputIt>::difference_type FindEdgeDistance(
		bool const rising,
		typename std::iterator_traits<InputIt>::value_type const threshold,
		InputIt const begin, InputIt const end) {

	auto const i = FindEdge(rising, threshold, begin, end);
	return end == i ? 0 : std::distance(begin, i);

}

}

#endif	//	MATH_EdgeFinder_hxx
