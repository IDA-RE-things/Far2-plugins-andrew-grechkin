#include <libbase/std.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/str.hpp>

namespace Base {

	astring w2cp(PCWSTR in, UINT cp) {
		size_t size = convert_cp(in, cp);
		auto_array<CHAR> buf(size);
		convert_cp(in, cp, buf.data(), size);
		return astring(buf.data());
	}

	ustring cp2w(PCSTR in, UINT cp) {
		size_t size = convert_cp(in, cp);
		auto_array<WCHAR> buf(size);
		convert_cp(in, cp, buf.data(), size);
		return ustring(buf.data());
	}

}
