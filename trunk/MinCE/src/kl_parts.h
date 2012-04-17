#ifndef KL_PARTS
#define KL_PARTS

#include <libwin_def/win_def.h>
#include <windows.h>
#include <tchar.h>

typedef unsigned int uint;
typedef unsigned __int8 byte;
typedef unsigned __int64 QWORD;
typedef __int8 i8;
typedef __int16 i16;
typedef __int32 i32;
typedef __int64 i64;
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int64 u64;
typedef DWORD u32;

template <class T> const T& klMin(const T &a, const T &b) {
	return a < b ? a : b;
}
template <class T> const T& klMax(const T &a, const T &b) {
	return a > b ? a : b;
}

#define klstricmp lstrcmpi
#define klstricmpA lstrcmpiA
#define klstricmpW lstrcmpiW

#ifdef UNICODE
#define klatoi _wtoi
//#define klSplitCommandLine klSplitCommandLineW
#define klstrncpy(in1, in2, in3) _tcsncpy((wchar_t *) in1, (wchar_t *) in2, in3)
#endif

#define sizeofarray(array) (sizeof(array)/sizeof(*array))
#define klInit()

struct klFile
			// define KL_File_no_init to skip generation of constructors and destructors
{
	HANDLE hHandle;
	u32 iCount;
	i64 lSize;
	TCHAR Name[MAX_PATH];

	void Close() {
		if (hHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(hHandle);
			hHandle = INVALID_HANDLE_VALUE;
		}
	}
	bool Open(const void *pName = nullptr, u32 Access = GENERIC_READ, u32 CreationDisposition = OPEN_EXISTING, u32 ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE, u32 FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL) {
		if (pName == nullptr)
			pName = Name;
		hHandle = CreateFile((LPCTSTR)pName, Access, ShareMode, nullptr, CreationDisposition, FlagsAndAttributes, nullptr);
		if (hHandle != INVALID_HANDLE_VALUE) {
			if (pName != Name)
				klstrncpy(Name, pName, MAX_PATH);
			*(u32*)&lSize = GetFileSize(hHandle, (u32*) & lSize + 1);
			if ((u32)lSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
				lSize = 0;
			return true;
		} else
			return false;
	}
	bool Read(void *buffer, u32 lSize) {
		return ReadFile(hHandle, buffer, lSize, &iCount, nullptr) != 0 ? iCount == lSize : false;
	}
	bool Write(const void *buffer, u32 lSize) {
		return WriteFile(hHandle, buffer, lSize, &iCount, nullptr) != 0;
	}
};

inline void klSplitCommandLine(wchar_t *str, uint *n) {
	*n = 0;
	wchar_t *dst = str, ts;
	while (*str == ' ')
		str++;
	ts = ' ';
	while (*str) {
		if (*str == '"') {
			ts ^= 2; // ' ' <-> '"'
			str++;
		}
		while (*str && *str != '"' && *str != ts)
			*dst++ = *str++;
		if (*str == '"')
			continue;
		while (*str == ' ')
			str++;
		*dst++ = 0;
		(*n)++;
	}
	return;
}

inline u32 __cdecl klMulDivU32(u32 a, u32 b, u32 c) {
	return	a * b / c;
}

#endif
