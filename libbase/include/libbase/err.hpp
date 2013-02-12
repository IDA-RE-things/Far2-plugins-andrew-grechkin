#ifndef _LIBBASE_ERR_HPP_
#define _LIBBASE_ERR_HPP_

#include <libbase/std.hpp>

namespace Base {

	ustring ErrAsStr(DWORD err = ::GetLastError(), PCWSTR lib = nullptr);

	ustring NTStatusAsStr(ULONG status);

	ustring ErrAsStrWmi(HRESULT err);

}

#endif
