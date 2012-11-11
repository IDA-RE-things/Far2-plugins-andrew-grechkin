#include <libcom/std.hpp>
#include <libext/exception.hpp>


namespace {

	struct Initializer {
		~Initializer();

		Initializer();
	};

	Initializer::~Initializer() {
		::CoUninitialize();
	}

	Initializer::Initializer() {
		CheckCom(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
		CheckCom(::CoInitializeSecurity(
			nullptr,
			-1,		// COM negotiates service
			nullptr,// Authentication services
			nullptr,// Reserved
			RPC_C_AUTHN_LEVEL_PKT_PRIVACY,// authentication
			RPC_C_IMP_LEVEL_IMPERSONATE,// Impersonation
			nullptr,// Authentication info
			EOAC_STATIC_CLOAKING,// Additional capabilities
			nullptr// Reserved
		));
	}

}

namespace Com {

	void init() {
		static Initializer com;
	}

	HRESULT ConvertBoolToHRESULT(bool result) {
		if (result)
			return S_OK;
		DWORD lastError = ::GetLastError();
		if (lastError == 0)
			return E_FAIL;
		return HRESULT_FROM_WIN32(lastError);
	}
}
