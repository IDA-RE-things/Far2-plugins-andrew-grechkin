#include "win_def.h"

#include <shlwapi.h>

CStrW			Canonicalize(const CStrW &path) {
	CStrW	Result(MAX_PATH_LENGTH);
	::PathCanonicalizeW(Result.buffer(), path.c_str());
	return	Result;
}
CStrW			Expand(const CStrW &path) {
	DWORD	size = ::ExpandEnvironmentStringsW(path.c_str(), NULL, 0);
	if (size) {
		CStrW	Result(size);
		if (::ExpandEnvironmentStringsW(path.c_str(), Result.buffer(), Result.capacity()))
			return	Result;
	}
	return	CStrW();
}
CStrW			Validate(const CStrW &path) {
	CStrW	Result(path);
	return	Canonicalize(Expand(Result));
}
