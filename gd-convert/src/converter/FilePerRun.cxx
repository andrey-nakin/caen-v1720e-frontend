#include <fstream>
#include <iomanip>
#include <converter/FilePerRun.hxx>

namespace gdc {

namespace converter {

void FilePerRun::BeginRun(int /* transition */, int const run, int /* time */) {

	std::stringstream s;
	ConstructFileName(s, run);

	std::string filename = s.str();
	dest = std::unique_ptr < std::ostream
			> (new std::ofstream(filename, FileMode()));

}

void FilePerRun::EndRun(int /* transition */, int /* run */, int /* time */) {

	if (dest) {
		dest = nullptr;
	}

}

void FilePerRun::ProcessMidasEvent(TDataContainer& dataContainer) {

	ProcessMidasEvent(*dest, dataContainer);

}

void FilePerRun::ConstructFileName(std::ostream& dest, int const run) const {

	dest << "run" << std::setfill('0') << std::setw(5) << run
			<< FileExtension();

}

std::ios_base::openmode FilePerRun::FileMode() const {

	return std::ios_base::out;

}

}

}
