/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinStr, CStr, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable, WinCOM)
	@author		© 2010 Andrew Grechkin
	@link		(shlwapi, )
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#include "std.h"
#include <stdio.h>

#include <shlwapi.h>

extern "C" {
	WINBASEAPI ULONGLONG WINAPI	GetTickCount64();

//	_CRTIMP int __cdecl _snwprintf(wchar_t*, size_t, const wchar_t*, ...);
	_CRTIMP int __cdecl _vsnwprintf(wchar_t *_Dest, size_t _Count, const wchar_t *_Format, va_list _Args);
}

///===================================================================================== definitions
CStrA		Hash2Str(const PBYTE buf, size_t size);
CStrA		Hash2StrNum(const PBYTE buf, size_t size);
bool		Str2Hash(const CStrA &str, PVOID &hash, ULONG &size);

UINT		CheckUnicode(const PVOID buf, size_t size);
UINT		IsUTF8(const PVOID buf, size_t size);
bool		GetCP(HANDLE hFile, UINT &cp, bool bUseHeuristics = false);

AutoUTF		AsStr(const SYSTEMTIME &in, bool tolocal = true);
AutoUTF		AsStr(const FILETIME &in);

AutoUTF		CopyAfterLast(const AutoUTF &in, const AutoUTF &delim);

AutoUTF&	Cut(AutoUTF &inout, const AutoUTF &in);
bool		Cut(AutoUTF &inout, ssize_t &num, int base = 10);
AutoUTF&	CutAfter(AutoUTF &inout, const AutoUTF &delim);
AutoUTF&	CutBefore(AutoUTF &inout, const AutoUTF &delim);

AutoUTF&	ToLower(AutoUTF &inout);
AutoUTF		ToLowerOut(const AutoUTF &in);
AutoUTF&	ToUpper(AutoUTF &inout);
AutoUTF		ToUpperOut(const AutoUTF &in);

///=========================================================================================== Types
template <typename Type>
struct		NamedValues {
	PCWSTR	name;
	Type	value;

	static	PCWSTR	GetName(NamedValues<Type> dim[], size_t size, const Type &in) {
		for (size_t i = 0; i < size; ++i) {
			if (dim[i].value == in) {
				return	dim[i].name;
			}
		}
		return	L"unknown";
	}
};

class		MyUI64 {
	ULARGE_INTEGER	m_data;
public:
	MyUI64(uint64_t in = 0ull) {
		m_data.QuadPart = in;
	}
	MyUI64(DWORD low, DWORD high) {
		m_data.LowPart = low;
		m_data.HighPart = high;
	}
	MyUI64(const ULARGE_INTEGER &in) {
		m_data.QuadPart = in.QuadPart;
	}
	operator	uint64_t() const {
		return	m_data.QuadPart;
	}
};

///======================================================================================= WinBuffer
/// Обертка буфера
template<typename Type>
class		WinBuf {
	Type*		m_buf;
	size_t		m_size;

	void			Free() {
		WinMem::Free<Type*>(m_buf);
	}
public:
	~WinBuf() {
		Free();
	}
	WinBuf(): m_buf(nullptr), m_size(0) {
	}
	WinBuf(size_t size, bool bytes = false): m_buf(nullptr), m_size((bytes) ? size : size * sizeof(Type)) {
		WinMem::Alloc(m_buf, m_size);
	}
	WinBuf(const WinBuf &in): m_buf(nullptr), m_size(in.m_size) {
		if (WinMem::Alloc(m_buf, m_size)) {
			WinMem::Copy(m_buf, in.m_buf, m_size);
		}
	}
	const WinBuf	&operator=(const WinBuf & in) {
		if (this != &in) {
			Free();
			m_size = in.m_size;
			if (WinMem::Alloc(m_buf, m_size)) {
				WinMem::Copy(m_buf, in.m_buf, m_size);
			}
		}
		return	*this;
	}

	operator		Type*() {
		return	m_buf;
	}
	Type*			data() const {
		return	m_buf;
	}
	Type*			operator->() const {
		return	m_buf;
	}

	size_t			size() const {
		return	m_size;
	}
	size_t			capacity() const {
		return	m_size / sizeof(Type);
	}
	bool			reserve(size_t size, bool bytes = false) {
		if (!bytes) {
			size *= sizeof(Type);
		}
		if (m_size < size) {
			m_size = size;
			return	WinMem::Realloc(m_buf, m_size);
		}
		return	false;
	}
	void			zero() {
		WinMem::Zero(m_buf, m_size);
	}
};

///=================================================================================================
class		CStrMW {
	class	MzsData {
		PWSTR		m_data;
		size_t		m_capa;
		size_t		m_size;
	public:
		~MzsData() {
			delete[]	m_data;
		}
		explicit	MzsData(PCWSTR in): m_size(0) {
			if (in) {
				PCWSTR	ptr = in;
				while (*ptr) {
					ptr += (Len(ptr) + 1);
					++m_size;
				}
				m_capa = ptr - in + 1;
				m_data = new WCHAR[m_capa];
				WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
			} else {
				m_capa = 1;
				m_data = new WCHAR[m_capa];
				m_data[0] = 0;
			}
		}
		friend class CStrMW;
	};
	winstd::shared_ptr<MzsData>	m_str;
public:
	CStrMW(PCWSTR in = L""): m_str(new MzsData(in)) {
	}
	const CStrMW	&operator=(const CStrMW &in) {
		m_str = in.m_str;
		return	*this;
	}

	size_t			size() const {
		return	m_str->m_size;
	}
	size_t			capacity() const {
		return	m_str->m_capa;
	}
	PCWSTR			c_str() const {
		return	m_str->m_data;
	}
	PCWSTR			operator[](int index) const {
		PCWSTR	ptr = c_str();
		int		cnt = 0;
		while (*ptr && (cnt++ < index)) {
			ptr += (Len(ptr) + 1);
		}
		return	ptr;
	}
};

///========================================================================================= BitMask
template<typename Type>
struct		BitMask {
	static Type		FromStr(const AutoUTF &in, size_t lim = 0) {
		// count bits from 1
		Type	Result = 0;
		intmax_t	bit = 0;
		AutoUTF	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(--bit))
				WinBit::Set(Result, bit);
		}
		return	Result;
	}
	static Type		FromStr0(const AutoUTF &in, size_t lim = 0) {
		// count bits from zero
		Type	Result = 0;
		ssize_t	bit = 0;
		AutoUTF	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(bit))
				WinBit::Set(Result, bit);
		}
		return	Result;
	}

	static AutoUTF	AsStr(Type in, size_t lim = 0) {
		// count bits from 1
		AutoUTF	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(Num2Str(bit + 1), L",");
			}
		}
		return	Result;
	}
	static AutoUTF	AsStr0(Type in, size_t lim = 0) {
		// count bits from zero
		AutoUTF	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t	bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(Num2Str(bit), L",");
			}
		}
		return	Result;
	}
	static AutoUTF	AsStrBin(Type in, size_t lim = 0) {
		AutoUTF	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			Result += WinFlag::Check(in, (Type)flag) ? L'1' : L'0';
			flag >>= 1;
		}
		return	Result;
	}
	static AutoUTF	AsStrNum(Type in, size_t lim = 0) {
		AutoUTF	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			if (WinFlag::Check(in, (Type)flag)) {
				Result.Add(Num2Str(flag), L",");
			}
			flag >>= 1;
		}
		return	Result;
	}
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ console
///==================================================================================== ConsoleColor
struct		ConsoleColor {
	~ConsoleColor() {
		ColorRestore();
	}
	ConsoleColor(WORD color): m_color(0) {
		if (color && ColorSave())
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
private:
	bool	ColorSave() {
		CONSOLE_SCREEN_BUFFER_INFO tmp;
		if (::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &tmp)) {
			m_color = tmp.wAttributes;
		}
		return	m_color;
	}
	void	ColorRestore() {
		if (m_color)
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), m_color);
	}
	WORD	m_color;
};

///===================================================================================== Console out
inline int	vsnprintf(PWSTR buf, size_t len, PCWSTR format, va_list vl) {
	buf[len - 1] = 0;
	return ::_vsnwprintf(buf, len - 1, format, vl);
}
inline int	stdvprintf(DWORD nStdHandle, PCWSTR format, va_list vl) {
	auto_array<WCHAR> buf(64 * 1024);
	vsnprintf(buf, buf.size(), format, vl);
	return consoleout(buf, nStdHandle);
}
int			stdprintf(DWORD nStdHandle, PCWSTR format, ...);

inline int	printf(PCWSTR format, ...) {
	va_list vl;
	va_start(vl, format);
	int Result = stdvprintf(STD_OUTPUT_HANDLE, format, vl);
	va_end(vl);
	return Result;
}

inline int	vprintf(PCWSTR format, va_list vl) {
	return	stdvprintf(STD_OUTPUT_HANDLE, format, vl);
}
int			snprintf(PWSTR buff, size_t len, PCWSTR format, ...);
void		errx(int eval, PCSTR format, ...);

enum		WinLogLevel {
	LOG_TRACE =	-3,
	LOG_DEBUG,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_ERROR,
};

extern int	logLevel;
void		setLogLevel(WinLogLevel lvl);
void		logError(PCWSTR format, ...);
void		logError(DWORD errNumber, PCWSTR format, ...);
void		logDebug(PCWSTR format, ...);
void		logVerbose(PCWSTR format, ...);
void		logCounter(PCWSTR format, ...);
void		logInfo(PCWSTR format, ...);
void		logFile(WIN32_FIND_DATA FileData);

///========================================================================================== WinEnv
namespace	WinEnv {
	AutoUTF	Get(PCWSTR name);
	bool	Set(PCWSTR name, PCWSTR val);
	bool	Add(PCWSTR name, PCWSTR val);
	bool	Del(PCWSTR name);
}

inline DWORD	UserLogon(HANDLE &hToken, PCWSTR name, PCWSTR pass, DWORD type, PCWSTR dom = L"") {
	DWORD	Result = NO_ERROR;
	if (!::LogonUserW((PWSTR)name, (PWSTR)dom, (PWSTR)pass, type, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		Result = ::GetLastError();
	}
	return	Result;
}

///============================================================================================ path
inline AutoUTF	Canonicalize(PCWSTR path) {
	auto_array<WCHAR> ret(MAX_PATH_LEN);
	if (::PathCanonicalizeW(ret, path))
		return AutoUTF(ret);
	return	AutoUTF();
}
inline AutoUTF	Canonicalize(const AutoUTF &path) {
	return	Canonicalize(path.c_str());
}

inline AutoUTF	Expand(PCWSTR path) {
	auto_array<WCHAR> ret(MAX_PATH_LEN);
	if (::ExpandEnvironmentStringsW(path, ret, ret.size()))
		return	AutoUTF(ret);
	return	AutoUTF();
}
inline AutoUTF	Expand(const AutoUTF &path) {
	return	Expand(path.c_str());
}

inline AutoUTF	PathNice(PCWSTR path) {
	return	Canonicalize(Expand(path));
}
inline AutoUTF	PathNice(const AutoUTF &path) {
	return	Canonicalize(Expand(path.c_str()));
}

inline AutoUTF	path_compact(PCWSTR path, size_t size) {
	WCHAR	ret[MAX_PATH_LEN];
	if (::PathCompactPathExW(ret, path, size, 0))
		return	AutoUTF(ret);
	return AutoUTF();
}
inline AutoUTF	path_compact(const AutoUTF &path, size_t size) {
	return	path_compact(path.c_str(), size);
}

inline AutoUTF& ensure_end_path_separator(AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C) {
	if (!path.empty() && path.at(path.size() - 1) != sep) {
		path += sep;
	}
	return path;
}
inline AutoUTF& ensure_no_end_path_separator(AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C) {
	if (!path.empty() && path.at(path.size() - 1) != sep) {
		path.erase(path.size() - 1);
	}
	return path;
}

AutoUTF			Secure(PCWSTR path);
AutoUTF			Secure(const AutoUTF &path);
AutoUTF			UnExpand(PCWSTR path);
AutoUTF			UnExpand(const AutoUTF &path);
AutoUTF			Validate(PCWSTR path);
AutoUTF			Validate(const AutoUTF &path);

inline AutoUTF	SlashAdd(PCWSTR path, WCHAR add = PATH_SEPARATOR_C) {
	AutoUTF	Result(path);
	if (!Empty(path) && Result[Result.size() - 1] != L'\\')
		Result += add;
	return	Result;
}
inline AutoUTF	SlashAdd(const AutoUTF &path, WCHAR add = PATH_SEPARATOR_C) {
	return	SlashAdd(path.c_str(), add);
}
AutoUTF			SlashAddNec(PCWSTR path);
AutoUTF			SlashAddNec(const AutoUTF &path);
AutoUTF			SlashDel(PCWSTR path);
AutoUTF			SlashDel(const AutoUTF &path);

bool			IsPathUnix(PCWSTR path);
bool			IsPathUnix(const AutoUTF &path);

AutoUTF			ExtractFile(PCWSTR path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF			ExtractFile(const AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF			ExtractPath(PCWSTR path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF			ExtractPath(const AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF			GetSpecialPath(int csidl, bool create = true);

inline AutoUTF	MakePath(PCWSTR path, PCWSTR name) {
	AutoUTF	Result(PathNice(SlashAdd(path)));
	return AddWordEx(Result, name, PATH_SEPARATOR);
}
inline AutoUTF	MakePath(const AutoUTF &path, const AutoUTF &name) {
	return	MakePath(path.c_str(), name.c_str());
}

AutoUTF			PathUnix(PCWSTR path);
AutoUTF			PathUnix(const AutoUTF &path);
AutoUTF			PathWin(PCWSTR path);
AutoUTF			PathWin(const AutoUTF &path);

AutoUTF			GetWorkDirectory();
bool			SetWorkDirectory(PCWSTR path);
inline bool		SetWorkDirectory(const AutoUTF &path) {
	return	SetWorkDirectory(path.c_str());
}

#include <wchar.h>
inline AutoUTF add_trailing_slash(const AutoUTF& path) {
	if ((path.size() == 0) || (path[path.size() - 1] == L'\\')) {
		return path;
	} else {
		return path + L"\\";
	}
}

inline bool substr_match(const AutoUTF& str, AutoUTF::size_type pos, PCWSTR mstr) {
	size_t mstr_len = Len(mstr);
	if ((pos > str.size()) || (pos + mstr_len > str.size())) {
		return false;
	}
	return wmemcmp(str.c_str() + pos, mstr, mstr_len) == 0;
}

inline void locate_path_root(const AutoUTF& path, size_t& path_root_len, bool& is_unc_path) {
	unsigned prefix_len = 0;
	is_unc_path = false;
	if (substr_match(path, 0, L"\\\\")) {
		if (substr_match(path, 2, L"?\\UNC\\")) {
			prefix_len = 8;
			is_unc_path = true;
		} else if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
			prefix_len = 4;
		} else {
			prefix_len = 2;
			is_unc_path = true;
		}
	}
	if ((prefix_len == 0) && !substr_match(path, 1, L":\\")) {
		path_root_len = 0;
	} else {
		AutoUTF::size_type p = path.find(L'\\', prefix_len);
		if (p == AutoUTF::npos) {
			p = path.size();
		}
		if (is_unc_path) {
			p = path.find(L'\\', p + 1);
			if (p == AutoUTF::npos) {
				p = path.size();
			}
		}
		path_root_len = p;
	}
}

inline AutoUTF extract_file_name(const AutoUTF& path) {
	size_t pos = path.rfind(L"\\");
	if (pos == AutoUTF::npos) {
		pos = 0;
	} else {
		pos++;
	}
	size_t path_root_len;
	bool is_unc_path;
	locate_path_root(path, path_root_len, is_unc_path);
	if ((pos <= path_root_len) && (path_root_len != 0))
		return AutoUTF();
	else
		return path.substr(pos);
}
///========================================================================================= SysPath
namespace	SysPath {
	AutoUTF	Winnt();
	AutoUTF	Sys32();
	AutoUTF	SysNative();
	AutoUTF	InetSrv();
	AutoUTF	Dns();
	AutoUTF	Temp();

	AutoUTF	Users();
}

///========================================================================================== SysApp
namespace	SysApp {
	AutoUTF	appcmd();
	AutoUTF	dnscmd();
}

///===================================================================================== File system
/// Работа с файловой системой (неокончено)
typedef		WIN32_FILE_ATTRIBUTE_DATA	FileInfo;

class		WinFilePos {
	LARGE_INTEGER	m_pos;
public:
	WinFilePos(LARGE_INTEGER pos): m_pos(pos) {
	}
	WinFilePos(LONGLONG pos = 0LL) {
		m_pos.QuadPart = pos;
	}
	operator		LONGLONG() const {
		return	m_pos.QuadPart;
	}
	operator		LARGE_INTEGER() const {
		return	m_pos;
	}
	operator		PLARGE_INTEGER() const {
		return	(PLARGE_INTEGER)&m_pos;
	}
};

inline bool			FileOpenRead(PCWSTR	path, HANDLE &hFile) {
	hFile = ::CreateFileW(path, FILE_READ_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr, OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	return	hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool			FileOpenAttr(PCWSTR	path, HANDLE &hFile) {
	hFile = ::CreateFileW(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	return	hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool			FileClose(HANDLE hFile) {
	return	::CloseHandle(hFile) != 0;
}

inline AutoUTF		TempDir() {
	WCHAR	buf[MAX_PATH];
	buf[0] = 0;
	::GetTempPathW(sizeofa(buf), buf);
	return	AutoUTF(buf);
}
inline AutoUTF		TempFile(PCWSTR path) {
	WCHAR	buf[MAX_PATH];
	WCHAR	pid[32];
	buf[0] = 0;
	Num2Str(pid, ::GetCurrentProcessId());
	::GetTempFileNameW(path, pid, 0, buf);
	return	buf;
}
inline AutoUTF		TempFile(const AutoUTF &path) {
	return	TempFile(path.c_str());
}
inline AutoUTF		FullPath(PCWSTR path) {
	size_t	len = ::GetFullPathNameW(path, 0, nullptr, nullptr);
	if (len) {
		WCHAR	buf[len];
		::GetFullPathNameW(path, sizeofa(buf), buf, nullptr);
		return	AutoUTF(buf);
	}
	return	AutoUTF();
}

inline FileInfo		FileGetInfo(PCWSTR	path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info;
}
inline uint64_t		FileSize(PCWSTR	path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	((uint64_t)info.nFileSizeHigh) << 32 | info.nFileSizeLow;
}
inline uint64_t		FileSize(HANDLE	hFile) {
	LARGE_INTEGER	size;
	::GetFileSizeEx(hFile, &size);
	return	size.QuadPart;
}
inline bool			FilePos(HANDLE hFile, const WinFilePos &pos, DWORD m = FILE_BEGIN) {
	return	::SetFilePointerEx(hFile, pos, nullptr, m) != 0;
}
inline LONGLONG		FilePos(HANDLE hFile) {
	WinFilePos	pos;
	return	::SetFilePointerEx(hFile, WinFilePos(0LL), pos, FILE_CURRENT);
}

inline FILETIME		FileTimeCr(PCWSTR path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info.ftCreationTime;
}
inline FILETIME		FileTimeAc(PCWSTR path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info.ftLastAccessTime;
}
inline FILETIME		FileTimeWr(PCWSTR path) {
	FileInfo	info;
	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
	return	info.ftLastWriteTime;
}

inline bool is_valid_filename(PCWSTR name) {
	return	!(Eq(name, L".") || Eq(name, L"..") || Eq(name, L"..."));
}
inline bool is_valid_filename(const AutoUTF &name) {
	return	is_valid_filename(name.c_str());
}

inline DWORD get_attributes(PCWSTR path) {
	return	::GetFileAttributesW(path);
}
inline bool set_attributes(PCWSTR path, DWORD attr) {
	return	::SetFileAttributesW(path, attr) != 0;
}

inline bool is_exists(PCWSTR path) {
	return	::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}
inline bool is_exists(const AutoUTF &path) {
	return	is_exists(path.c_str());
}
inline bool file_exists(PCWSTR path) {
	return	::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}
inline bool file_exists(const AutoUTF &path) {
	return	is_exists(path.c_str());
}

inline bool is_file(PCWSTR path) {
	DWORD	attr = ::GetFileAttributesW(path);
	return attr != INVALID_FILE_ATTRIBUTES && 0 == (attr & FILE_ATTRIBUTE_DIRECTORY);
}
inline bool is_file(const AutoUTF &path) {
	return	is_file(path.c_str());
}

inline bool is_dir(PCWSTR path) {
	DWORD	attr = ::GetFileAttributesW(path);
	return attr != INVALID_FILE_ATTRIBUTES && 0 != (attr & FILE_ATTRIBUTE_DIRECTORY);
}
inline bool is_dir(const AutoUTF &path) {
	return	is_dir(path.c_str());
}

inline bool dir_is_empty(PCWSTR path) {
	return	::PathIsDirectoryEmptyW(path);
}
inline bool dir_is_empty(const AutoUTF &path) {
	return	dir_is_empty(path.c_str());
}

extern "C" {
	INT WINAPI		SHCreateDirectoryExA(HWND, PCSTR, PSECURITY_ATTRIBUTES);
	INT WINAPI		SHCreateDirectoryExW(HWND, PCWSTR, PSECURITY_ATTRIBUTES);
}

inline bool create_dir(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return	::SHCreateDirectoryExW(nullptr, path, lpsa) == ERROR_SUCCESS;
}
inline bool create_dir(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return create_dir(path.c_str(), lpsa);
}

inline bool create_file(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	HANDLE hFile = ::CreateFileW(path, 0, 0, lpsa, OPEN_ALWAYS, 0, nullptr);
	::CloseHandle(hFile);
	return hFile && hFile != INVALID_HANDLE_VALUE;
}
inline bool create_file(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return create_file(path.c_str(), lpsa);
}

inline bool create_hardlink(PCWSTR path, PCWSTR newfile) {
	return	::CreateHardLinkW(newfile, path, nullptr) != 0;
}
inline bool create_hardlink(const AutoUTF &path, const AutoUTF &newfile) {
	return	create_hardlink(path.c_str(), newfile.c_str());
}

inline bool ensure_dir_exist(PCWSTR path) {
	if (is_exists(path) && is_dir(path))
		return true;
	return	create_dir(path);
}
inline bool ensure_dir_exist(const AutoUTF &path) {
	return	ensure_dir_exist(path.c_str());
}

inline bool is_path_mask(PCWSTR path) {
	PCWSTR	pos = find_last_of(path, L"?*");
	return	(pos && pos != (path + 2));
}
inline bool is_path_mask(const AutoUTF &path) {
	return is_path_mask(path.c_str());
}

inline AutoUTF remove_path_prefix(const AutoUTF &path, PCWSTR pref = PATH_PREFIX_NT) {
	if (path.find(pref) == 0)
		return path.substr(Len(pref));
	return	path;
}

inline AutoUTF ensure_path_prefix(const AutoUTF &path, PCWSTR pref = PATH_PREFIX_NT) {
	if (path.find(pref) != 0)
		return AutoUTF(pref) + path;
	return	path;
}

inline AutoUTF	get_path_from_mask(const AutoUTF &mask) {
	AutoUTF	tmp(remove_path_prefix(mask));
	AutoUTF::size_type pos = std::min(tmp.find_first_of(L"?*"), tmp.find_last_of(L"\\/"));
	if (pos != AutoUTF::npos) {
		tmp.erase(pos);
	}
	return	ensure_path_prefix(tmp);
}

inline bool delete_dir(PCWSTR path) {
	::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
	return	::RemoveDirectoryW(path) != 0;
}
inline bool delete_dir(const AutoUTF &path) {
	return	delete_dir(path.c_str());
}

inline bool delete_file(PCWSTR path) {
	DWORD	attr = get_attributes(path);
	if (::SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL)) {
		if (::DeleteFileW(path)) {
			return	true;
		}
		::SetFileAttributesW(path, attr);
	}
	return	false;
}
inline bool delete_file(const AutoUTF &path) {
	return	delete_file(path.c_str());
}

inline bool delete_sh(PCWSTR path) {
	SHFILEOPSTRUCTW sh;

	sh.hwnd = nullptr;
	sh.wFunc = FO_DELETE;
	sh.pFrom = path;
	sh.pTo = nullptr;
	sh.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	sh.hNameMappings = 0;
	sh.lpszProgressTitle = nullptr;
	::SHFileOperationW(&sh);
	return	true;
}

inline bool delete_on_reboot(PCWSTR path) {
	return	::MoveFileExW(path, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
}
inline bool delete_on_reboot(const AutoUTF &path) {
	return	delete_on_reboot(path.c_str());
}

inline bool delete_recycle(PCWSTR path) {
	SHFILEOPSTRUCTW	info = {0};
	info.wFunc	= FO_DELETE;
	info.pFrom	= path;
	info.fFlags	= FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
	return	::SHFileOperationW(&info) == 0;
}
inline bool delete_recycle(const AutoUTF &path) {
	return	delete_recycle(path.c_str());
}

inline bool copy_file(PCWSTR path, PCWSTR dest) {
	return	::CopyFileW(path, dest, true) != 0;
}
inline bool copy_file(const AutoUTF &path, const AutoUTF &dest) {
	return	copy_file(path.c_str(), dest.c_str());
}

inline bool move_file(PCWSTR path, PCWSTR dest, DWORD flag = 0) {
	return	::MoveFileExW(path, dest, flag);
}
inline bool move_file(const AutoUTF &path, const AutoUTF &dest, DWORD flag = 0) {
	return	move_file(path.c_str(), dest.c_str(), flag);
}

inline bool read_file(HANDLE hFile, PBYTE buf, DWORD &size) {
	return	::ReadFile(hFile, buf, size, &size, nullptr) != 0;
}
bool read_file(PCWSTR path, CStrA &buf);

inline bool get_file_inode(PCWSTR path, uint64_t &inode, size_t &link) {
	HANDLE	hFile;
	if (FileOpenAttr(path, hFile)) {
		BY_HANDLE_FILE_INFORMATION	info;
		if (::GetFileInformationByHandle(hFile, &info)) {
			inode = info.nNumberOfLinks;
			link = info.nNumberOfLinks;
		}
		FileClose(hFile);
		return true;
	}
	return	false;
}

bool				FileCreate(PCWSTR path, PCWSTR name, PCSTR content);
inline bool		FileCreate(const AutoUTF &path, const AutoUTF &name, PCSTR content) {
	return	FileCreate(path.c_str(), name.c_str(), content);
}

bool				FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite = false);
inline bool		FileWrite(PCWSTR path, PCWSTR data, size_t size, bool rewrite = false) {
	return	FileWrite(path, data, size * sizeof(WCHAR), rewrite);
}
inline bool		FileWrite(const AutoUTF &path, const AutoUTF &data, bool rewrite = false) {
	return	FileWrite(path.c_str(), (PCVOID)data.c_str(), data.size() * sizeof(WCHAR), rewrite);
}
inline size_t		FileWrite(HANDLE file, const PVOID &in, size_t size) {
	DWORD	Result = 0;
	::WriteFile(file, in, size, &Result, nullptr);
	return	Result;
}
inline size_t		FileWrite(HANDLE file, const AutoUTF &in) {
	return	FileWrite(file, (const PVOID)in.c_str(), in.size() * sizeof(WCHAR));
}

AutoUTF				GetDrives();

///============================================================================================ Link
bool is_link(PCWSTR path);
inline bool is_link(const AutoUTF &path) {
	return is_link(path.c_str());
}

bool is_symlink(PCWSTR path);
inline bool is_symlink(const AutoUTF &path) {
	return is_symlink(path.c_str());
}

bool is_junction(PCWSTR path);
inline bool is_junction(const AutoUTF &path) {
	return is_junction(path.c_str());
}

bool copy_link(PCWSTR from, PCWSTR to);
inline bool copy_link(const AutoUTF &from, const AutoUTF &to) {
	return copy_link(from, to);
}

bool create_link(PCWSTR link, PCWSTR dest);
inline bool	create_link(const AutoUTF &link, const AutoUTF &dest) {
	return create_link(link.c_str(), dest.c_str());
}

bool create_junc(PCWSTR path, PCWSTR dest);
inline bool	create_junc(const AutoUTF &path, const AutoUTF &dest) {
	return create_junc(path.c_str(), dest.c_str());
}

bool delete_link(PCWSTR path);
inline bool delete_link(const AutoUTF &path) {
	return delete_link(path.c_str());
}

bool break_link(PCWSTR path);
inline bool break_link(const AutoUTF &path) {
	return break_link(path.c_str());
}

AutoUTF	read_link(PCWSTR path);
inline AutoUTF read_link(const AutoUTF &path) {
	return read_link(path.c_str());
}

///========================================================================================= WinPriv
/// Функции работы с привилегиями
namespace	WinPriv {
	bool 		IsExist(HANDLE hToken, LUID priv);
	bool 		IsExist(HANDLE hToken, PCWSTR sPriv);
	bool 		IsExist(LUID priv);
	bool 		IsExist(PCWSTR sPriv);

	bool		IsEnabled(HANDLE hToken, LUID priv);
	bool 		IsEnabled(HANDLE hToken, PCWSTR sPriv);
	bool		IsEnabled(LUID priv);
	bool 		IsEnabled(PCWSTR sPriv);

	bool 		Modify(HANDLE hToken, LUID priv, bool bEnable);
	bool 		Modify(HANDLE hToken, PCWSTR sPriv, bool bEnable);
	bool 		Modify(LUID priv, bool bEnable);
	bool 		Modify(PCWSTR sPriv, bool bEnable);

	inline bool	Disable(LUID in) {
		return	Modify(in, false);
	}
	inline bool	Disable(PCWSTR in) {
		return	Modify(in, false);
	}

	inline bool	Enable(LUID in) {
		return	Modify(in, true);
	}
	inline bool	Enable(PCWSTR in) {
		return	Modify(in, true);
	}

	AutoUTF		GetName(PCWSTR sPriv);
}

///======================================================================================= Privilege
class Privilege: private Uncopyable {
public:
	~Privilege();

	explicit Privilege(PCWSTR name);

private:
	TOKEN_PRIVILEGES m_tp;
	bool	m_disable;
};

///========================================================================================= WinProc
/// Обертка хэндла процесса
class		WinProcess: private Uncopyable, public WinErrorCheck {
	HANDLE	m_hndl;
public:
	~WinProcess() {
		::CloseHandle(m_hndl);
	}
	WinProcess() {
		m_hndl = ::GetCurrentProcess();
	}
	WinProcess(ACCESS_MASK mask, DWORD pid): m_hndl(nullptr) {
		m_hndl = ::OpenProcess(mask, false, pid);
		ChkSucc(m_hndl != nullptr);
	}
	operator		HANDLE() const {
		return	m_hndl;
	}
	DWORD			GetId() const {
		return	::GetProcessId(m_hndl);
	}

// static
	static	DWORD	Id() {
		return	::GetCurrentProcessId();
	}
	static	DWORD	Id(HANDLE hProc) {
		return	::GetProcessId(hProc);
	}
	static	AutoUTF	User();
	static	AutoUTF	FullPath();
	static	AutoUTF	CmdLine() {
		return	::GetCommandLineW();
	}
};

///======================================================================================== WinToken
/// Обертка токена
class		WinToken: private Uncopyable, public WinErrorCheck {
	HANDLE	m_handle;
public:
	~WinToken() {
		::CloseHandle(m_handle);
	}
	WinToken(ACCESS_MASK mask = TOKEN_ALL_ACCESS): m_handle(nullptr) {
		ChkSucc(::OpenProcessToken(WinProcess(), mask, &m_handle) != 0);
	}
	WinToken(ACCESS_MASK mask, HANDLE hProcess): m_handle(nullptr) {
		ChkSucc(::OpenProcessToken(hProcess, mask, &m_handle) != 0);
	}
	operator		HANDLE() const {
		return	m_handle;
	}

	static bool		CheckMembership(PSID sid, HANDLE hToken = nullptr) {
		BOOL	Result;
		::CheckTokenMembership(hToken, sid, &Result);
		return	Result;
	}
};

///========================================================================================= WinTime
struct		WinTime: public FILETIME {
	void			Init(const ULARGE_INTEGER &in) {
		dwLowDateTime	= in.LowPart;
		dwHighDateTime	= in.HighPart;
	}
public:
	WinTime() {
		now();
	}
	WinTime(const uint64_t &in) {
		ULARGE_INTEGER tmp;
		tmp.QuadPart = in;
		Init(tmp);
	}
	WinTime(const ULARGE_INTEGER &in) {
		Init(in);
	}
	operator		FILETIME() const {
		return	*this;
	}
	operator		ULARGE_INTEGER() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return	Result;
	}
	operator		uint64_t() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return	Result.QuadPart;
	}
	void			now() {
		::GetSystemTimeAsFileTime(this);
	}

	const WinTime&	operator=(const ULARGE_INTEGER & in) {
		Init(in);
		return	*this;
	}
	WinTime&		operator+=(const uint64_t & in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart += in * Second();
		Init(tmp);
		return	*this;
	}
	WinTime&		operator-=(const uint64_t & in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart -= in * Second();
		Init(tmp);
		return	*this;
	}
	WinTime			operator+(const uint64_t &in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart += in * Second();
		return	WinTime(tmp);
	}
	uint64_t		operator-(const WinTime &in) {
		ULARGE_INTEGER tmp = *this;
		tmp.QuadPart -= ((ULARGE_INTEGER)in).QuadPart;
		return	tmp.QuadPart / Second();
	}

	static uint64_t	MiliSecond() {
		return	10000ULL;
	}
	static uint64_t	Second() {
		return	10000000ULL;
	}
	static uint64_t	Minute() {
		return	600000000ULL;
	}
	static uint64_t	Hour() {
		return	36000000000ULL;
	}
	static uint64_t	Day() {
		return	864000000000ULL;
	}
	static uint64_t	Week() {
		return	6048000000000ULL;
	}
	static uint64_t	SecPerDay() {
		return	60ULL * 60 * 24;
	}
	static uint64_t	SecPerHour() {
		return	60ULL * 60;
	}
};

///========================================================================================= WinTime
class		WinSysTime: public SYSTEMTIME {
public:
	WinSysTime() {
		WinMem::Zero(*this);
		Now();
	}
	void			Now(bool isLocal = false) {
		if (isLocal)
			::GetLocalTime(this);
		else
			::GetSystemTime(this);
	}
	void			AddYear(long in) {
		wYear += in;
	}
};

///======================================================================================== WinTimer
/// Оконный таймер
class		WinTimer {
	HANDLE			hTimer;
	LARGE_INTEGER	liUTC;
	long			lPeriod;

	void			Open();
	void			Close();
public:
	~WinTimer();
	WinTimer();
	WinTimer(LONGLONG time, long period = 0);
	void			Set(LONGLONG time, long period = 0);
	void			Start();
	void			Stop();
	void			StartTimer();
	operator		HANDLE() const {
		return	hTimer;
	}
};

///=========================================================================================== Win64
/// Функции работы с WOW64
namespace	Win64 {
	bool	WowDisable(PVOID &oldValue);
	bool	WowEnable(PVOID &oldValue);
	bool	IsWOW64();
}

///====================================================================================== WinSysInfo
struct		WinSysInfo: public SYSTEM_INFO {
	WinSysInfo();
	size_t			Uptime(size_t del = 1000);
};

#endif // WIN_DEF_HPP
