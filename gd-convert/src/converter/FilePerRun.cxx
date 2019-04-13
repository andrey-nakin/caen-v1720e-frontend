#include <fstream>
#include <iomanip>
#include <cstring>
#include <converter/FilePerRun.hxx>

namespace gdc {

namespace converter {

namespace cmd {

constexpr char fileName[] = "-F";
constexpr char stdout[] = "stdout";

}

FilePerRun::FilePerRun() :
		run(-1) {

}

void FilePerRun::BeginRun(int /* transition */, int const run, int /* time */) {

	this->run = run;

	std::string filename;

	if (fixedFileName.empty()) {
		std::stringstream s;
		ConstructFileName(s, run);
		filename = s.str();
	} else {
		filename = fixedFileName;
	}

	useStdout = filename == cmd::stdout;
	if (!useStdout) {
		dest = std::unique_ptr < std::ostream
				> (new std::ofstream(filename, FileMode()));
	} else {
		std::ios::sync_with_stdio(false);
	}

}

void FilePerRun::EndRun(int /* transition */, int /* run */, int /* time */) {

	if (dest) {
		dest = nullptr;
	}

}

bool FilePerRun::ProcessMidasEvent(TDataContainer& dataContainer) {

	return ProcessMidasEvent(useStdout ? std::cout : *dest, dataContainer);

}

void FilePerRun::Configure(std::vector<char*>& args) {

	for (std::size_t i = 0; i < args.size();) {
		if (StartsWith(args[i], cmd::fileName)) {
			fixedFileName = std::string(args[i] + std::strlen(cmd::fileName));
		} else {
			i++;
			continue;
		}
		args.erase(args.begin() + i);
	}

}

void FilePerRun::ConstructFileName(std::ostream& dest, int const run) const {

	dest << "run" << std::setfill('0') << std::setw(5) << run
			<< FileExtension();

}

std::ios_base::openmode FilePerRun::FileMode() const {

	return std::ios_base::out;

}

int FilePerRun::GetRun() const {

	return run;

}

}

}
