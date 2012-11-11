#include <libcom/unknown.hpp>


namespace Com {

	UnknownImp::~UnknownImp() {
	}

	UnknownImp::UnknownImp() :
		m_ref_cnt(1) {
	}

	ULONG WINAPI UnknownImp::AddRef() {
		return ++m_ref_cnt;
	}

	ULONG WINAPI UnknownImp::Release() {
		if (--m_ref_cnt == 0) {
			delete this;
			return 0;
		}
		return m_ref_cnt;
	}

	HRESULT WINAPI UnknownImp::QueryInterface(REFIID riid, void ** ppvObject) {
		if (riid == IID_IUnknown) {
			*ppvObject = static_cast<IUnknown*>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

}
