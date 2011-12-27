#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

namespace SevenZip {
	///================================================================================ OpenCallback
	OpenCallback::~OpenCallback() {
	}

	OpenCallback::OpenCallback() {
	}

	ULONG WINAPI OpenCallback::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG WINAPI OpenCallback::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI OpenCallback::QueryInterface(REFIID riid, void ** object) {
		UNKNOWN_IMPL_ITF(IArchiveOpenCallback)
		UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
		return UnknownImp::QueryInterface(riid, object);
	}

	HRESULT WINAPI OpenCallback::SetTotal(const UInt64 */*files*/, const UInt64 */*bytes*/) {
		return S_OK;
	}

	HRESULT WINAPI OpenCallback::SetCompleted(const UInt64 */*files*/, const UInt64 */*bytes*/) {
		return S_OK;
	}

	HRESULT WINAPI OpenCallback::CryptoGetTextPassword(BSTR */*password*/) {
		printf(L"%S\n", __PRETTY_FUNCTION__);
		if (Password.empty()) {
			// You can ask real password here from user
			// Password = GetPassword(OutStream);
			// PasswordIsDefined = true;
			return E_ABORT;
		}
		return S_OK;
	}
}
