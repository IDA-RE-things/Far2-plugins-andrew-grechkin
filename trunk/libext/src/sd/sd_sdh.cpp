#include <libext/sd.hpp>
#include <libext/exception.hpp>

namespace Ext {

	///====================================================================================== WinSDH
	WinSDH::~WinSDH() {
	}

	void WinSDH::Get() {
		free(m_sd);
		CheckApiError(::GetSecurityInfo(m_hnd, m_type,
		                                ALL_SD_INFO,
		                                nullptr, nullptr, nullptr, nullptr, &m_sd));
	}

	void WinSDH::Set() const {
		set_security(m_hnd, m_sd, m_type);
	}

}
