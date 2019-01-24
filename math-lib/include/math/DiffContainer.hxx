#ifndef MATH_DiffContainer_hxx
#define	MATH_DiffContainer_hxx

#include <iterator>
#include <utility>

namespace math {

template<class ValueT, class InputIt>
class DiffContainer {
public:

	class Iterator {
	public:

		using iterator_category = typename std::iterator_traits<InputIt>::iterator_category;
		typedef ValueT value_type;
		typedef ValueT* pointer;
		typedef ValueT& reference;
		using difference_type = typename std::iterator_traits<InputIt>::difference_type;

		Iterator(InputIt const aFirst, InputIt const aLast) :
				first(aFirst), last(aLast) {
		}

		bool operator==(Iterator const i) const {

			return last == i.last;

		}

		bool operator!=(Iterator const i) const {

			return last != i.last;

		}

		value_type operator*() const {

			return static_cast<value_type>(*last)
					- static_cast<value_type>(*first);

		}

		template<class Distance>
		Iterator& operator+=(Distance const d) {

			std::advance(first, d);
			std::advance(last, d);
			return *this;

		}

		Iterator& operator++() {

			++first;
			++last;
			return *this;

		}

		difference_type operator-(Iterator const i) const {

			return std::distance(i.last, last);

		}

	private:

		InputIt first, last;

	};

	DiffContainer(InputIt const aBegin, InputIt const aEnd,
			std::size_t const aDistance) :
			distance(aDistance), range(MakeRange(aBegin, aEnd, aDistance)) {

	}

	Iterator begin() const {

		return range.first == range.second ?
				end() : Iterator(range.first, std::next(range.first, distance));

	}

	Iterator end() const {

		return Iterator(range.second, range.second);

	}

private:

	std::size_t const distance;
	std::pair<InputIt, InputIt> const range;

	static std::pair<InputIt, InputIt> MakeRange(InputIt const aBegin,
			InputIt const aEnd, std::size_t const aDistance) {

		if (std::distance(aBegin, aEnd)
				>= static_cast<typename Iterator::difference_type>(aDistance)) {
			return std::pair<InputIt, InputIt>(aBegin, aEnd);
		} else {
			return std::pair<InputIt, InputIt>(aEnd, aEnd);
		}

	}

};

template<class ValueT, class InputIt>
DiffContainer<ValueT, InputIt> MakeDiffContainer(InputIt const begin,
		InputIt const end, std::size_t const shift) {

	return DiffContainer<ValueT, InputIt>(begin, end, shift);

}

}

#endif	//	MATH_DiffContainer_hxx
