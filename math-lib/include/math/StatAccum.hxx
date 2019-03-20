#ifndef MATH_StatAccum_hxx
#define	MATH_StatAccum_hxx

#include <iterator>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace math {

template<class AccumT, class InputIt>
class StatAccum {
public:

	typedef std::uint_fast32_t counter_type;
	typedef typename std::iterator_traits<InputIt>::value_type value_type;
	typedef AccumT accum_type;

	StatAccum() :
			counter(0), sum(0), sum2(0) {

	}

	void operator()(value_type const v) {
		if (0 == counter++) {
			minv = maxv = v;
		} else {
			if (minv > v) {
				minv = v;
			}
			if (maxv < v) {
				maxv = v;
			}
		}
		sum += v;
		sum2 += v * v;
	}

	bool operator==(StatAccum const& oth) const {

		return counter == oth.counter && sum == oth.sum && sum2 == oth.sum2
				&& minv == oth.minv && maxv == oth.maxv;

	}

	value_type GetRoughMean() const {

		return static_cast<value_type>((sum + counter / 2) / counter);

	}

	double GetMean() const {

		return static_cast<double>(sum) / counter;

	}

	accum_type GetRoughVariance() const {

		if (counter > 1) {
			accum_type const divisor = counter * (counter - 1);
			return (counter * sum2 - sum * sum + divisor / 2) / divisor;
		} else {
			return 0;
		}

	}

	double GetVariance() const {

		if (counter > 1) {
			return static_cast<double>(counter * sum2 - sum * sum)
					/ (counter * (counter - 1));
		} else {
			return 0;
		}

	}

	double GetStd() const {

		return std::sqrt(GetVariance());

	}

	template<class ResultT = value_type>
	ResultT GetStdScaled(double const scale) const {

		return static_cast<ResultT>(GetStd() * scale + .5);

	}

	counter_type GetCount() const {

		return counter;

	}

	value_type GetMinValue() const {

		return minv;

	}

	value_type GetMaxValue() const {

		return maxv;

	}

private:

	counter_type counter;
	accum_type sum, sum2;
	value_type minv, maxv;

};

template<class AccumT = int64_t, class InputIt>
StatAccum<AccumT, InputIt> MakeStatAccum(InputIt const begin,
		InputIt const end) {

	return std::distance(begin, end) > 0 ?
			std::for_each(begin, end, StatAccum<AccumT, InputIt>()) :
			StatAccum<AccumT, InputIt>();

}

template<class AccumT = int64_t, class InputIt>
StatAccum<AccumT, InputIt> MakeStatAccum(InputIt const begin, InputIt const end,
		StatAccum<AccumT, InputIt> const init) {

	return std::distance(begin, end) > 0 ?
			std::for_each(begin, end, init) : init;

}

template<class AccumT = int64_t, class InputIt>
StatAccum<AccumT, InputIt> MakeStatAccum(InputIt const begin, InputIt const end,
		InputIt const excludeBegin, InputIt const excludeEnd) {

	return MakeStatAccum(excludeEnd, end, MakeStatAccum(begin, excludeBegin));

}

}

#endif	//	MATH_StatAccum_hxx
