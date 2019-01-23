#ifndef MATH_DiffContainer_hxx
#define	MATH_DiffContainer_hxx

#include <iterator>

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

			return first == i.first;

		}

		bool operator!=(Iterator const i) const {

			return first != i.first;

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

		difference_type operator-(Iterator const i) const {

			return std::distance(i.first, first);

		}

	private:

		InputIt first, last;

	};

	DiffContainer(InputIt const aBegin, InputIt const aEnd,
			std::size_t const aShift) :
			shift(aShift) {

		if (std::distance(aBegin, aEnd)
				>= static_cast<typename Iterator::difference_type>(aShift)) {
			from = aBegin;
			to = aEnd;
			empty = false;
		} else {
			from = to = aEnd;
			empty = true;
		}

	}

	Iterator begin() {

		return empty ? Iterator(to, to) : Iterator(from, from + shift);

	}

	Iterator end() {

		return empty ? Iterator(to, to) : Iterator(to - shift, to);

	}

private:

	std::size_t const shift;
	InputIt from, to;
	bool empty;

};

template<class ValueT, class InputIt>
DiffContainer<ValueT, InputIt> MakeDiffContainer(InputIt const begin,
		InputIt const end, std::size_t const shift) {

	return DiffContainer<ValueT, InputIt>(begin, end, shift);

}

}

#endif	//	MATH_DiffContainer_hxx
