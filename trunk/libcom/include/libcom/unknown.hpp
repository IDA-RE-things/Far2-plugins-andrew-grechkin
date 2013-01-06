#ifndef _LIBCOM_UNKNOWN_HPP_
#define _LIBCOM_UNKNOWN_HPP_

#include <libbase/std.hpp>

namespace Com {

	struct UnknownImp: public IUnknown {
		virtual ~UnknownImp();

		UnknownImp();

		ULONG WINAPI AddRef() override;

		ULONG WINAPI Release() override;

		HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObject) override;

	private:
		ULONG m_ref_cnt;
	};

}

#endif
