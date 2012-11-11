#ifndef _LIBCOM_UNKNOWN_HPP_
#define _LIBCOM_UNKNOWN_HPP_

#include <libbase/std.hpp>


namespace Com {

	struct UnknownImp: public IUnknown {
		virtual ~UnknownImp();

		UnknownImp();

		virtual ULONG WINAPI AddRef();

		virtual ULONG WINAPI Release();

		virtual HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject);

	private:
		ULONG m_ref_cnt;
	};

}


#endif
