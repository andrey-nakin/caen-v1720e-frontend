#include <caen/v1720.hxx>
#include <midas/odb.hxx>
#include <fe-v1720.hxx>
#include <util/V1720InfoRawData.hxx>
#include <util/TWaveFormRawData.hxx>
#include "hist/V1720Waveform.hxx"

namespace bwf {

namespace hist {

V1720Waveform::V1720Waveform(VirtualOdb* const anOdb) :
		AbstractWaveform(anOdb, fe::v1720::equipName, fe::v1720::displayName,
				caen::v1720::nsPerSample<ns_per_sample_type>()) {

}

V1720Waveform::~V1720Waveform() {
}

void V1720Waveform::UpdateHistograms(TDataContainer &dataContainer) {

	using util::V1720InfoRawData;
	using util::TWaveFormRawData;

	auto const v1720Info = dataContainer.GetEventData < V1720InfoRawData
			> (V1720InfoRawData::bankName());
	if (v1720Info) {
		for (unsigned channelNo = 0; channelNo < 8; channelNo++) {
			if (v1720Info->channelIncluded(channelNo)) {
				auto const wfRaw = dataContainer.GetEventData < TWaveFormRawData
						> (TWaveFormRawData::bankName(channelNo));
				if (wfRaw) {
					auto const h = GetHist(
							frontendIndex(v1720Info->info().frontendIndex),
							channelNo);
					if (h) {
						auto const numOfSamples = wfRaw->numOfSamples();
						auto s = wfRaw->waveForm();

						h->Reset();
						for (unsigned i = 0; i < numOfSamples; i++) {
							h->SetBinContent(i + 1, *s++);
						}
					}
				}
			}
		}

	}
}

unsigned V1720Waveform::loadWaveformLength(INT const feIndex) {

	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
			fe::v1720::settings::waveformLength);

	if (getOdb()->odbReadArraySize(hKeyName.c_str()) <= 0) {
		return 0;
	}

	return getOdb()->odbReadUint32(hKeyName.c_str(), 0, 0);

}

std::vector<bool> V1720Waveform::loadEnabledChannels(INT feIndex) {

	std::vector<bool> result;
	auto const hKeyName = odb::equipSettingsKeyName(getBaseEquipName(), feIndex,
			fe::v1720::settings::enabledChannels);

	auto const size = getOdb()->odbReadArraySize(hKeyName.c_str());
	if (size > 0) {
		result.resize(size);

		for (int i = 0; i < size; i++) {
			result[i] = getOdb()->odbReadBool(hKeyName.c_str(), i, false);
		}
	}

	return result;

}

}

}
