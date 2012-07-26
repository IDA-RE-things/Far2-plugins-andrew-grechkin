#include <libext/sid.hpp>
#include <libext/exception.hpp>


namespace Ext {

	Sid::size_type Sid::size() const {
		return this_type::size(m_sid);
	}

	Sid::size_type Sid::size(value_type in) {
		return check(in), ::GetLengthSid(in);
	}

}
