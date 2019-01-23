#ifndef MATH_DiffContainer_hxx
#define	MATH_DiffContainer_hxx

#include <iterator>

namespace math {

template<class InputIt, class ValueT>
class DiffContainer {
public:

//	class Iterator {
//	public:
//		using iterator_category = std::input_iterator_tag;
//		typedef ValueT value_type;
//
//		Iterator(InputIt const aFirst, InputIt const aLast) :
//				first(aFirst), last(aLast) {
//		}
//
//		bool operator==(Iterator const i) const {
//
//			return first == i.first;
//
//		}
//
//		bool operator!=(Iterator const i) const {
//
//			return first != i.first;
//
//		}
//
//		value_type operator*() const {
//
//			return static_cast<value_type>(*last)
//					- static_cast<value_type>(*first);
//
//		}
//
//		Iterator& operator++() const {
//
//			++first;
//			++last;
//			return *this;
//
//		}
//
//		Iterator& operator++(int) const {
//
//			first++;
//			last++;
//			return *this;
//
//		}
//
//		Iterator& operator--() const {
//
//			--first;
//			--last;
//			return *this;
//
//		}
//
//		Iterator& operator--(int) const {
//
//			first--;
//			last--;
//			return *this;
//
//		}
//
//	private:
//		InputIt const first, last;
//
//	};

	typedef ValueT value_type;

	DiffContainer(InputIt const aBegin, InputIt const aEnd,
			std::size_t const aShift) :
			begin(aBegin), end(aEnd), shift(aShift) {
	}

//	Iterator begin() {
//
//		return Iterator(begin, begin + shift);
//
//	}
//
//	Iterator end() {
//
//		return Iterator(end - shift, end);
//
//	}

private:

	InputIt const begin, end;
	std::size_t shift;

};


}

#endif	//	MATH_DiffContainer_hxx
