#include <libext/sid.hpp>
#include <libext/exception.hpp>

namespace Ext {

	bool Sid::is_valid() const {
		return this_type::is_valid(m_sid);
	}

	bool Sid::is_valid(value_type in) {
		return in && ::IsValidSid(in);
	}

}
