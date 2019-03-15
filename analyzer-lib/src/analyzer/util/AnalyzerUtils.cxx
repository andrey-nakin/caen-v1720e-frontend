#include <analyzer/util/AnalyzerUtils.hxx>

namespace analyzer {

namespace util {

std::string AnalyzerUtils::OdbRootKey(std::string const& analyzerName) {

	return std::string("/Analyzer/") + analyzerName;

}

std::string AnalyzerUtils::OdbKey(std::string const& rootKeyName,
		std::string const& keyName) {

	return rootKeyName + "/" + keyName;

}

}

}
