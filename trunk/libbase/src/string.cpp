#include <libbase/std.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/str.hpp>

//#include <array>

namespace Base {

	astring w2cp(PCWSTR in, UINT cp) {
		auto_array<CHAR> buf(Str::convert(in, cp));
		Str::convert(buf.data(), buf.size(), in, cp);
		return astring(&buf[0]);
	}

	ustring cp2w(PCSTR in, UINT cp) {
		auto_array<wchar_t> buf(Str::convert(in, cp));
		Str::convert(buf.data(), buf.size(), in, cp);
		return ustring(buf.data());
	}

}
