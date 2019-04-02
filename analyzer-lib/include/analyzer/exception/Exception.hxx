#pragma once

#include <exception>
#include <sstream>
#include <memory>

namespace analyzer {

namespace exception {

class Exception: public std::exception {
public:

	Exception();
	Exception(Exception const&);
	~Exception() override;

	template<typename ValueT>
	Exception& operator<<(ValueT const& v) {
		msgStream << v;
		return *this;
	}

	const char* what() const throw () override {
		if (!msg) {
			msg.reset(new std::string(msgStream.str()));
		}
		return msg->c_str();
	}

private:

	mutable std::unique_ptr<std::string> msg;
	std::stringstream msgStream;

};

}

}
