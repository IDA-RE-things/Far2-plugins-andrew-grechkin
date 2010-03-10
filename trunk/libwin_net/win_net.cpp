#include "win_net.h"

///========================================================================================== WinNet
namespace	WinNet {
bool		GetCompName(CStrW &buf, COMPUTER_NAME_FORMAT cnf) {
	DWORD	size = 0;
	::GetComputerNameExW(cnf, NULL, &size);
	buf.reserve(size);
	return	::GetComputerNameExW(cnf, buf.buffer(), &size) != 0;
}
}
