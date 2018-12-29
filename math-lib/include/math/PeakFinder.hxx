#ifndef MATH_Peakfinder_hxx
#define	MATH_Peakfinder_hxx

#include <iterator>

namespace math {

template<class InputIt>
class PeakFinder {

	enum class State {

		Init, UnderLower, AboveUpper

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

		FindPeak();
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

	void FindPeak() {

		for (; i != last; i++) {
			if (*i <= lowerThreshold) {
				switch (state) {
				case State::Init:
				case State::AboveUpper:
					state = State::UnderLower;
					peak = i;
					break;

				case State::UnderLower:
					if (*i < *peak) {
						peak = i;
					}
					break;
				}
			} else if (*i > upperThreshold) {
				switch (state) {
				case State::UnderLower:
					state = State::AboveUpper;
					peakMode = Mode::Falling;
					return;
				}

			}
		}

	}

};

}

#endif	//	MATH_Peakfinder_hxx
