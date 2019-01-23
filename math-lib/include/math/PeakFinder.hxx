#ifndef MATH_Peakfinder_hxx
#define	MATH_Peakfinder_hxx

#include <iterator>
#include <functional>

namespace math {

/*
template<class InputIt, class AccumT = std::int64_t>
class StatAccum {
public:
	typedef typename std::iterator_traits<InputIt>::value_type value_type;
	typedef typename std::iterator_traits<InputIt>::difference_type difference_type;

	StatAccum(InputIt const begin, InputIt const end) :
			counter(0), sum(0), sum2(0) {

		std::for_each(begin, end, *this);

	}

	void operator(value_type const v) {
		counter++;
		sum += v;
		sum2 += v * v;
	}

	AccumT GetVariance() const {

	}

	value_type GetStd() const {

		return std::sqrt(GetVariance());

	}

private:

	std::uint_fast32_t counter;
	AccumT sum, sum2;

};
*/

template<class InputIt>
class PeakFinder {
/*
	enum class State {

		Init, Middle, UnderLower, AboveUpper

	};

public:

	enum class Mode {

		Rising, Falling

	};

	typedef typename std::iterator_traits<InputIt>::value_type value_type;
	typedef typename std::iterator_traits<InputIt>::difference_type difference_type;

	PeakFinder(Mode const aMode, InputIt const begin, InputIt const end,
			difference_type const aFrontLength,
			difference_type const aPeakLength, value_type const aThreshold) :
			mode(aMode), peak(end), first(begin), last(end), frontLength(
					aFrontLength), peakLength(aPeakLength), threshold(
					aThreshold), state(State::Init) {

		if (std::distance(begin, end) < aFrontLength) {
			i = end;
		} else {
			i = std::next(begin, aFrontLength);
		}

	}

	bool Next() {

		switch (mode) {
		case Mode::Rising:
			FindPeak(std::greater<value_type>(), upperThreshold,
					lowerThreshold);
			peakMode = Mode::Rising;
			break;

		case Mode::Falling:
			FindPeak(std::less<value_type>(), lowerThreshold, upperThreshold);
			peakMode = Mode::Falling;
			break;
		}
		return i != last;

	}

	value_type GetValue() const {

		return peak == last ? value_type() : *peak;

	}

	difference_type GetPosition() const {

		return peak == last ? 0 : std::distance(first, peak);

	}

	Mode GetType() const {

		return peakMode;

	}

private:

	Mode const mode;
	InputIt i, peak;
	InputIt const first, last;
	value_type const threshold;
	difference_type const frontLength, peakLength;
	State state;
	Mode peakMode;

	void FindPeak(std::function<bool(value_type, value_type)> comparator,
			value_type const aLimit, value_type const bLimit) {

		auto prev = i - frontLength;
		for (; i != last; i++, prev++) {
			auto const edge = *i - *prev;
			switch (mode) {
			case Mode::Falling:
				if (-edge >= threshold) {

				}
				break;
			}
		}

	}
*/
};

}

#endif	//	MATH_Peakfinder_hxx
