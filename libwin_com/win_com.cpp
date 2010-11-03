#include "win_com.h"

///========================================================================================== WinCom
WinCOM::~WinCOM() {
	::CoUninitialize();
}

WinCOM& WinCOM::init() {
	static WinCOM com;
	return com;
}

WinCOM::WinCOM() {
	CheckError(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
}
