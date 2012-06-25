#ifndef _LIBBASE_TIME_HPP_
#define _LIBBASE_TIME_HPP_

#include <libbase/std.hpp>
#include <libbase/memory.hpp>

namespace Base {

	struct WinFileTime: public FILETIME {
		WinFileTime() {
			now();
		}
		WinFileTime(const uint64_t & in) {
			ULARGE_INTEGER tmp;
			tmp.QuadPart = in;
			Init(tmp);
		}
		WinFileTime(const ULARGE_INTEGER &in) {
			Init(in);
		}
		operator FILETIME() const {
			return *this;
		}
		operator ULARGE_INTEGER() const {
			ULARGE_INTEGER Result;
			Result.LowPart = this->dwLowDateTime;
			Result.HighPart = this->dwHighDateTime;
			return Result;
		}
		operator uint64_t() const {
			ULARGE_INTEGER Result;
			Result.LowPart = this->dwLowDateTime;
			Result.HighPart = this->dwHighDateTime;
			return Result.QuadPart;
		}
		void now() {
			::GetSystemTimeAsFileTime(this);
		}

		const WinFileTime& operator=(const ULARGE_INTEGER & in) {
			Init(in);
			return *this;
		}
		WinFileTime& operator+=(const uint64_t & in) {
			ULARGE_INTEGER tmp = *this;
			tmp.QuadPart += in * Second();
			Init(tmp);
			return *this;
		}
		WinFileTime& operator-=(const uint64_t & in) {
			ULARGE_INTEGER tmp = *this;
			tmp.QuadPart -= in * Second();
			Init(tmp);
			return *this;
		}
		WinFileTime operator+(const uint64_t &in) {
			ULARGE_INTEGER tmp = *this;
			tmp.QuadPart += in * Second();
			return WinFileTime(tmp);
		}
		uint64_t operator-(const WinFileTime &in) {
			ULARGE_INTEGER tmp = *this;
			tmp.QuadPart -= ((ULARGE_INTEGER)in).QuadPart;
			return tmp.QuadPart / Second();
		}

		static uint64_t MiliSecond() {
			return 10000ULL;
		}
		static uint64_t Second() {
			return 10000000ULL;
		}
		static uint64_t Minute() {
			return 600000000ULL;
		}
		static uint64_t Hour() {
			return 36000000000ULL;
		}
		static uint64_t Day() {
			return 864000000000ULL;
		}
		static uint64_t Week() {
			return 6048000000000ULL;
		}
		static uint64_t SecPerDay() {
			return 60ULL * 60 * 24;
		}
		static uint64_t SecPerHour() {
			return 60ULL * 60;
		}

	private:
		void Init(const ULARGE_INTEGER &in) {
			dwLowDateTime = in.LowPart;
			dwHighDateTime = in.HighPart;
		}
	};

	///========================================================================================= WinTime
	struct WinSysTime: public SYSTEMTIME {
		WinSysTime() {
			Memory::zero(this, sizeof(*this));
			now();
		}

		void now(bool isLocal = false) {
			if (isLocal)
				::GetLocalTime(this);
			else
				::GetSystemTime(this);
		}

		void add_year(size_t in) {
			wYear += in;
		}
	};

	///======================================================================================== WinTimer
	/// Оконный таймер
	class WinTimer {
		HANDLE hTimer;
		LARGE_INTEGER liUTC;
		long lPeriod;

		void Open();
		void Close();
	public:
		~WinTimer();
		WinTimer();
		WinTimer(LONGLONG time, long period = 0);
		void Set(LONGLONG time, long period = 0);
		void Start();
		void Stop();
		void StartTimer();
		operator HANDLE() const {
			return hTimer;
		}
	};

}

#endif
