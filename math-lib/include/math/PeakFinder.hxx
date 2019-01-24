#ifndef MATH_Peakfinder_hxx
#define	MATH_Peakfinder_hxx

#include <iterator>
#include <algorithm>

namespace math {

template<class InputIt>
class PeakFinder {
public:

	typedef InputIt iterator_type;
	using value_type = typename std::iterator_traits<InputIt>::value_type;
	using difference_type = typename std::iterator_traits<InputIt>::difference_type;

	PeakFinder(bool const aRising, InputIt const aBegin, InputIt const aEnd,
			difference_type const aFrontLength, value_type const aThreshold,
			difference_type const aPeakLength) :
			rising(aRising), from(aBegin), to(aEnd), frontLength(aFrontLength), threshold(
					aThreshold), peakLength(aPeakLength) {

	}

	bool HasNext() {

		peak = FindNext(from);
		return peak != to;

	}

	iterator_type GetNext() {

		return peak;

	}

private:

	bool const rising;
	iterator_type from, peak;
	iterator_type const to;
	difference_type const frontLength;
	value_type const threshold;
	difference_type const peakLength;

	iterator_type FindNext(iterator_type& from) {

		if (std::distance(from, to) >= frontLength) {
			for (iterator_type i = from + frontLength, j = from; i != to;
					++i, ++j) {
				auto const exceeds =
						rising ? *i - *j >= threshold : *j - *i >= threshold;
				if (exceeds) {
					from = j + std::min(peakLength, std::distance(j, to));
					return rising ?
							std::max_element(j, from) :
							std::min_element(j, from);
				}
			}
		}

		from = to;
		return to;

	}

};

template<class InputIt>
PeakFinder<InputIt> MakePeakFinder(bool const rising, InputIt const begin,
		InputIt const end,
		typename PeakFinder<InputIt>::difference_type const frontLength,
		typename PeakFinder<InputIt>::value_type const threshold,
		typename PeakFinder<InputIt>::difference_type const peakLength) {

	return PeakFinder<InputIt>(rising, begin, end, frontLength, threshold,
			peakLength);

}

}

#endif	//	MATH_Peakfinder_hxx
