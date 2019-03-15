#pragma once

#include <string>

namespace analyzer {

namespace util {

class AnalyzerUtils {
private:

	AnalyzerUtils() = delete;

public:

	static std::string OdbRootKey(std::string const& analyzerName);
	static std::string OdbKey(std::string const& rootKeyName, std::string const& keyName);

};

}

}
