#pragma once

#include <cstring>
#include <vector>

namespace util {

class VectorComparator {

	VectorComparator() = delete;

public:

	template<typename ValueT>
	static bool equal(std::vector<ValueT> const& a,
			std::vector<ValueT> const& b) {

		if (a.size() != b.size()) {
			return false;
		}

		if (a.empty()) {
			return true;
		}

		return 0 == memcmp(&a[0], &b[0], a.size() * sizeof(a[0]));

	}

	static bool equal(std::vector<bool> const& a, std::vector<bool> const& b) {

		return a == b;

	}

};

}
