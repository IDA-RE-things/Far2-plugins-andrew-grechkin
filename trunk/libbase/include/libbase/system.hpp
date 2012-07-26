#ifndef _LIBBASE_SYSTEM_HPP_
#define _LIBBASE_SYSTEM_HPP_

#include <libbase/std.hpp>

namespace Base {

	struct SecurityAttributes: SECURITY_ATTRIBUTES {
		SecurityAttributes(bool inherit = false, PVOID psd = nullptr) {
			nLength = sizeof(SECURITY_ATTRIBUTES);
			lpSecurityDescriptor = psd;
			bInheritHandle = inherit;
		}
	};

	struct StopWatch {
		StopWatch() {
			::QueryPerformanceFrequency(&m_liPerfFreq);
			Start();
		}

		void Start() {
			::QueryPerformanceCounter(&m_liPerfStart);
		}

		int64_t Now() const { // Returns # of milliseconds since Start was called
			LARGE_INTEGER liPerfNow;
			::QueryPerformanceCounter(&liPerfNow);
			return (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000) / m_liPerfFreq.QuadPart);
		}

	private:
		LARGE_INTEGER m_liPerfFreq;		// Counts per second
		LARGE_INTEGER m_liPerfStart;	// Starting count
	};



}



#endif
