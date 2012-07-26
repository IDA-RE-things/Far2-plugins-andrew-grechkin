#include <libext/sd.hpp>
#include <libext/exception.hpp>

#include <sddl.h>

namespace Ext {

	///=============================================================================== sddl_string_t
	sddl_string_t::sddl_string_t(const ustring & str):
		m_str(str) {
	}

	ustring sddl_string_t::as_str() const {
		return m_str;
	}

	ustring as_str(const sddl_string_t & sddl_str) {
		return sddl_str.as_str();
	}

	///===================================================================================== WinSDDL
	WinSDDL::~WinSDDL() {
	}

	WinSDDL::WinSDDL(PCWSTR sddl) {
		CheckApi(::ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl, SDDL_REVISION, &m_sd, nullptr));
	}

}
