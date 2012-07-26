#include <libext/sid.hpp>
#include <libext/exception.hpp>

namespace Ext {

	ustring Sid::get_name() const {
		return this_type::get_name(m_sid);
	}

	ustring Sid::get_domain() const {
		return this_type::get_domain(m_sid);
	}

	ustring Sid::get_name(value_type sid, PCWSTR srv) {
		ustring nam, dom;
		get_name_dom(sid, nam, dom, srv);
		return nam;
	}

	ustring Sid::get_domain(value_type sid, PCWSTR srv) {
		ustring nam, dom;
		get_name_dom(sid, nam, dom, srv);
		return dom;
	}

}
