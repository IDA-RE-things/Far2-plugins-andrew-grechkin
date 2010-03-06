#include "win_def.h"

namespace	WinEnv {
CStrW		Get(PCWSTR name) {
	CStrW	buf(::GetEnvironmentVariable(name, NULL, 0));
	::GetEnvironmentVariable(name, buf.buffer(), (DWORD)buf.capacity());
	return	buf;
}
bool		Set(PCWSTR name, PCWSTR val) {
	return	::SetEnvironmentVariable(name, val) != 0;
}
bool		Add(PCWSTR name, PCWSTR val) {
	CStrW	buf(::GetEnvironmentVariable(name, NULL, 0) + WinStr::Len(val));
	::GetEnvironmentVariable(name, buf.buffer(), (DWORD)buf.capacity());
	buf += val;
	return	::SetEnvironmentVariable(name, buf.c_str()) != 0;
}
bool		Del(PCWSTR name) {
	return	::SetEnvironmentVariable(name, NULL) != 0;
}
}
