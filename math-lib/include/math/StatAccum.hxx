#ifndef MATH_StatAccum_hxx
#define	MATH_StatAccum_hxx

#include <algorithm>

namespace math {

template<class AccumT, class InputIt>
class StatAccum {
public:

	typedef std::uint_fast32_t counter_type;
	typedef typename std::iterator_traits<InputIt>::value_type value_type;
	typedef typename std::iterator_traits<InputIt>::difference_type difference_type;

	StatAccum() :
			counter(0), sum(0), sum2(0) {

	}

	void operator()(value_type const v) {
		counter++;
		sum += v;
		sum2 += v * v;
	}

	AccumT GetVariance() const {

	}

	value_type GetStd() const {

		return std::sqrt(GetVariance());

	}

	counter_type GetCount() const {

		return counter;

	}

private:

	counter_type counter;
	AccumT sum, sum2;

};

template<class AccumT, class InputIt>
StatAccum<AccumT, InputIt> MakeStatAccum(InputIt const begin,
		InputIt const end) {

	return std::for_each(begin, end, StatAccum<AccumT, InputIt>());

}

}

#endif	//	MATH_StatAccum_hxx
