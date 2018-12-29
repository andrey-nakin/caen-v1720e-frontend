#ifndef MATH_Peakfinder_hxx
#define	MATH_Peakfinder_hxx

#include <iterator>
#include <functional>

namespace math {

template<class InputIt>
class PeakFinder {

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
			value_type const aLowerThreshold, value_type const anUpperThreshold) :
			mode(aMode), i(begin), peak(end), first(begin), last(end), lowerThreshold(
					aLowerThreshold), upperThreshold(anUpperThreshold), state(
					State::Init) {

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
	value_type const lowerThreshold, upperThreshold;
	State state;
	Mode peakMode;

	void FindPeak(std::function<bool(value_type, value_type)> comparator,
			value_type const aLimit, value_type const bLimit) {

		for (; i != last; i++) {
			if (comparator(*i, aLimit)) {
				switch (state) {
				case State::Init:
				case State::Middle:
				case State::AboveUpper:
					state = State::UnderLower;
					peak = i;
					break;

				case State::UnderLower:
					if (comparator(*i, *peak)) {
						peak = i;
					}
					break;
				}
			} else if (!comparator(*i, bLimit)) {
				switch (state) {
				case State::Init:
					state = State::AboveUpper;
					break;
				case State::UnderLower:
					state = State::AboveUpper;
					return;
				}

			} else {
				switch (state) {
				case State::Init:
					state = State::Middle;
					peak = i;
					break;
				}
			}
		}

	}

};

}

#endif	//	MATH_Peakfinder_hxx
