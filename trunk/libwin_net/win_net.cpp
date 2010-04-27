#include "win_net.h"

///========================================================================================== WinNet
namespace	WinNet {
AutoUTF		GetCompName(COMPUTER_NAME_FORMAT cnf) {
	DWORD	size = 0;
	::GetComputerNameExW(cnf, NULL, &size);
	WCHAR	buf[size];
	::GetComputerNameExW(cnf, buf, &size);
	return	buf;
}
}
