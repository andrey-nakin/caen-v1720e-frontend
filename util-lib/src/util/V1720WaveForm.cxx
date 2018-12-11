#include <algorithm>
#include <iterator>
#include "util/V1720WaveForm.hxx"

namespace util {

V1720WaveForm::V1720WaveForm(TWaveFormRawData const& rawData,
		uint16_t const dcOffset) {

	Float_t const multiplier = 2.0f / 4095.0f;
	Float_t const offset = (65535.0f - dcOffset) / 65536.0f * (-2.0f);
	std::transform(rawData.waveForm(),
			rawData.waveForm() + rawData.numOfSamples(),
			std::back_insert_iterator < std::vector < Float_t >> (voltages),
			[multiplier, offset](uint16_t s) {
				return multiplier * s + offset;
			});

	Float_t t = 0.0;
	times.resize(voltages.size());
	std::generate(times.begin(), times.end(), [&t] {
		auto const res = t;
		t += 4.0f;
		return res;
	});

}

}
