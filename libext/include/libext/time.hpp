#ifndef _WIN_TIME_H_
#define _WIN_TIME_H_

#include <libbase/std.hpp>

namespace Ext {

	class WinTime {
	public:
		size_t year() const {
			return m_time.wYear;
		}

		size_t month() const {
			return m_time.wMonth;
		}

		size_t day() const {
			return m_time.wDay;
		}

	protected:
		WinTime() {
		}

		SYSTEMTIME m_time;
	};

	class LocalTime: public WinTime {
	public:
		LocalTime() {
			::GetLocalTime(&m_time);
		}
	};

}

#endif
