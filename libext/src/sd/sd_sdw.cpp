#include <libext/sd.hpp>
#include <libext/exception.hpp>

namespace Ext {

	///====================================================================================== WinSDW
	WinSDW::~WinSDW() {
	}

	void WinSDW::Get() {
		free(m_sd);
		CheckApiError(::GetNamedSecurityInfoW((PWSTR)m_name.c_str(), m_type,
		                                      ALL_SD_INFO,
		                                      nullptr, nullptr, nullptr, nullptr, &m_sd));
	}

	void WinSDW::Set() const {
		set_security(m_name.c_str(), m_sd, m_type);
	}

	void WinSDW::Set(PCWSTR path) const {
		set_security(path, m_sd, m_type);
	}

}
