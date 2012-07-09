#include "exception_pvt.hpp"
#include <libbase/err.hpp>

namespace Ext {

	///================================================================================== HMailError
#ifndef NDEBUG
	HMailError::HMailError(HRESULT code, PCSTR file, size_t line, PCSTR func):
		WinError(code, file, line, func) {
	}
#else
	HMailError::HMailError(HRESULT code):
		WinError(code) {
	}
#endif

	HMailError * HMailError::clone() const {
		return new HMailError(*this);
	}

	ustring HMailError::type() const {
		return L"HMailError";
	}

	ustring	HMailError::what() const {
		return Base::ErrAsStr(code(), L"hMailServer.exe");
	}

#ifndef NDEBUG
	HRESULT HiddenFunctions::CheckHMailErrorFunc(HRESULT err, PCSTR file, size_t line, PCSTR func) {
		if (err != ERROR_SUCCESS) {
			throw HMailError(err, file, line, func);
		}
		return err;
	}
#else
	HRESULT HiddenFunctions::CheckHMailErrorFunc(HRESULT err) {
		if (err != ERROR_SUCCESS) {
			throw HMailError(err);
		}
		return err;
	}
#endif
}
