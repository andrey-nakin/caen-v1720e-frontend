#ifndef	UTIL_LIB_FrontEndUtils_hxx
#define	UTIL_LIB_FrontEndUtils_hxx

#include <cstdint>
#include <midas.h>
#include <caen/exception.hxx>

#ifndef NEED_NO_EXTERN_C
extern "C" {
#endif

INT get_frontend_index();

#ifndef NEED_NO_EXTERN_C
}
#endif

namespace util {

class FrontEndUtils {
public:

	static std::string settingsKeyName(const char* equipName,
			const char* propName = nullptr);

	static HNDLE settingsKey(const char* equipName);

	static INT handleCaenException(caen::Exception const& ex);

	template<class Function>
	static INT command(Function func) {

		int status = SUCCESS;

		try {

			func();

		} catch (midas::Exception& ex) {
			status = ex.getStatus();
		} catch (caen::Exception& ex) {
			status = handleCaenException(ex);
		}

		return status;

	}

	template<class Function, class Result = int>
	static Result commandR(Function func) {

		Result result;

		try {

			result = func();

		} catch (midas::Exception& ex) {
			result = 0;
		} catch (caen::Exception& ex) {
			handleCaenException(ex);
			result = 0;
		}

		return result;

	}

	template<typename IntT>
	static IntT frontendIndex() {

		auto const index = get_frontend_index();
		return index >= 0 ? static_cast<IntT>(index) : static_cast<IntT>(-1);

	}

private:

	FrontEndUtils() = delete;

};

}

#endif	//	UTIL_LIB_FrontEndUtils_hxx
