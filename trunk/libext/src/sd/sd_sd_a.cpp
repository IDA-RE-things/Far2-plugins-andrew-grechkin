#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libbase/logger.hpp>

namespace Ext {

	WinSD::WinSD(WinSD && rhs):
		m_sd(nullptr)
	{
		LogTrace();
		this->swap(rhs);
	}

	WinSD & WinSD::operator = (WinSD && rhs) {
		LogTrace();
		if (this != &rhs)
			this->swap(rhs);
//			WinSD(std::move(rhs)).swap(*this);
		return *this;
	}

	void WinSD::swap(WinSD & rhs) {
		LogTrace();
		using std::swap;
		swap(m_sd, rhs.m_sd);
	}

}
