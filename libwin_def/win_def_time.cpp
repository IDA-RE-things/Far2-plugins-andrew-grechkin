#include "win_def.h"

///======================================================================================== WinTimer
/// Оконный таймер
void			WinTimer::Open() {
	Close();
	hTimer = ::CreateWaitableTimer(null_ptr, false, null_ptr);
}
void			WinTimer::Close() {
	if (hTimer) {
		::CloseHandle(hTimer);
		hTimer = null_ptr;
	}
}

WinTimer::~WinTimer() {
	Stop();
	Close();
}
WinTimer::WinTimer(): hTimer(null_ptr) {
	Open();
	liUTC.QuadPart = 0LL;
	lPeriod = 0L;
}
WinTimer::WinTimer(LONGLONG time, long period): hTimer(null_ptr) {
	Open();
	Set(time, period);
}

void			WinTimer::Set(LONGLONG time, long period) {
	if (time == 0LL) {
		liUTC.QuadPart = -period * 10000LL;
	} else {
		liUTC.QuadPart = time;
	}
	lPeriod = period;
}
void			WinTimer::Start() {
	if (hTimer)
		::SetWaitableTimer(hTimer, &liUTC, lPeriod, null_ptr, null_ptr, false);
}
void			WinTimer::Stop() {
	if (hTimer)
		::CancelWaitableTimer(hTimer);
}
void			WinTimer::StartTimer() {
	// объявляем свои локальные переменные
	FILETIME ftLocal, ftUTC;
	LARGE_INTEGER liUTC;

// таймер должен сработать в первый раз 1 января 2002 года в 1:00 PM но местному времени
	SYSTEMTIME st;
	::GetSystemTime(&st);
//		st.wOayOfWeek = 0;			// игнорируется
//		st.wHour = 0;				// 0 PM
//		st.wMinute = 0;				// 0 минут
	st.wSecond = 0;				// 0 секунд
	st.wMilliseconds = 0;		// 0 миллисекунд
	::SystemTimeToFileTime(&st, &ftLocal);
//		::LocalFileTimeToFilelime(&ttLocal, &ftUTC);

// преобразуем FILETIME в LARGE_INTEGER из-за различий в выравнивании данных
	liUTC.LowPart = ftUTC.dwLowDateTime;
	liUTC.HighPart = ftUTC.dwHighDateTime;
}
