/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable)
	@author		© 2010 Andrew Grechkin
	@link		(shlwapi, )
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#include "std.h"
#include "shared_ptr.h"

#include <stdio.h>

#include <shlwapi.h>

extern "C" {
	INT WINAPI	SHCreateDirectoryExA(HWND, PCSTR, PSECURITY_ATTRIBUTES);
	INT WINAPI	SHCreateDirectoryExW(HWND, PCWSTR, PSECURITY_ATTRIBUTES);
//	WINBASEAPI ULONGLONG WINAPI	GetTickCount64();

//	_CRTIMP int __cdecl _snwprintf(wchar_t*, size_t, const wchar_t*, ...);
//	_CRTIMP int __cdecl _vsnwprintf(wchar_t *_Dest, size_t _Count, const wchar_t *_Format, va_list _Args);
}

///===================================================================================== definitions
astring		Hash2Str(const PBYTE buf, size_t size);
astring		Hash2StrNum(const PBYTE buf, size_t size);
bool		Str2Hash(const astring &str, PVOID &hash, ULONG &size);

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

///========================================================================================= mstring
class		mstring {
public:
	mstring(PCWSTR in = L""): m_str(new impl(in)) {
	}

	size_t	size() const {
		return	m_str->m_size;
	}

	size_t	capacity() const {
		return	m_str->m_capa;
	}

	PCWSTR	c_str() const {
		return	m_str->m_data;
	}

	PCWSTR	operator[](int index) const {
		PCWSTR	ptr = c_str();
		int		cnt = 0;
		while (*ptr && (cnt++ < index)) {
			ptr += (Len(ptr) + 1);
		}
		return	ptr;
	}

private:
	class	impl {
	public:
		~impl() {
			WinMem::Free(m_data);
		}

		explicit	impl(PCWSTR in): m_size(0) {
			if (!in)
				in = EMPTY_STR;
			PCWSTR	ptr = in;
			while (*ptr) {
				ptr += (Len(ptr) + 1);
				++m_size;
			}
			m_capa = ptr - in + 1;
			WinMem::Alloc(m_data, sizeof(WCHAR) * m_capa);
			WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
		}
	private:
		PWSTR	m_data;
		size_t	m_capa;
		size_t	m_size;

		friend class mstring;
	};

	winstd::shared_ptr<impl>	m_str;
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

inline void PrintString(const AutoUTF &str) {
	printf(L"%s\n", str.c_str());
}

///========================================================================================== WinEnv
namespace	WinEnv {
	AutoUTF	Get(PCWSTR name);
	bool	Set(PCWSTR name, PCWSTR val);
	bool	Add(PCWSTR name, PCWSTR val);
	bool	Del(PCWSTR name);
}

///======================================================================================== WinToken
/// Обертка токена
class		WinToken: private Uncopyable, public WinErrorCheck {
public:
	WinToken(ACCESS_MASK mask = TOKEN_ALL_ACCESS) {
		ChkSucc(::OpenProcessToken(::GetCurrentProcess(), mask, &m_hndl) != 0);
	}

	WinToken(HANDLE hProcess, ACCESS_MASK mask) {
		ChkSucc(::OpenProcessToken(hProcess, mask, &m_hndl) != 0);
	}

	operator		HANDLE() const {
		return	m_hndl;
	}

	static bool		CheckMembership(PSID sid, HANDLE hToken = nullptr) {
		BOOL	Result;
		::CheckTokenMembership(hToken, sid, &Result);
		return	Result;
	}
private:
	auto_close<HANDLE>	m_hndl;
};

inline DWORD	UserLogon(HANDLE &hToken, PCWSTR name, PCWSTR pass, DWORD type, PCWSTR dom = L"") {
	DWORD	Result = NO_ERROR;
	if (!::LogonUserW((PWSTR)name, (PWSTR)dom, (PWSTR)pass, type, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		Result = ::GetLastError();
	}
	return	Result;
}

///============================================================================================ path
AutoUTF			Canonicalize(PCWSTR path);
inline AutoUTF	Canonicalize(const AutoUTF &path) {
	return	Canonicalize(path.c_str());
}

AutoUTF			Expand(PCWSTR path);
inline AutoUTF	Expand(const AutoUTF &path) {
	return	Expand(path.c_str());
}

AutoUTF			UnExpand(PCWSTR path);
inline AutoUTF	UnExpand(const AutoUTF &path) {
	return	UnExpand(path.c_str());
}

AutoUTF 		MakeGoodPath(PCWSTR path);
inline AutoUTF MakeGoodPath(const AutoUTF path) {
	return MakeGoodPath(path.c_str());
}

AutoUTF			get_fullpath(PCWSTR path);
inline AutoUTF	get_fullpath(const AutoUTF &path) {
	return get_fullpath(path.c_str());
}

AutoUTF			PathNice(PCWSTR path);
inline AutoUTF	PathNice(const AutoUTF &path) {
	return	Canonicalize(Expand(path.c_str()));
}

AutoUTF			path_compact(PCWSTR path, size_t size);
inline AutoUTF	path_compact(const AutoUTF &path, size_t size) {
	return	path_compact(path.c_str(), size);
}

AutoUTF&		ensure_end_path_separator(AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C);

AutoUTF&		ensure_no_end_path_separator(AutoUTF &path);

AutoUTF			Secure(PCWSTR path);
inline AutoUTF	Secure(const AutoUTF &path) {
	return	Secure(path.c_str());
}

AutoUTF			Validate(PCWSTR path);
inline AutoUTF	Validate(const AutoUTF &path) {
	return	Validate(path.c_str());
}

AutoUTF			SlashAdd(const AutoUTF &path, WCHAR sep = PATH_SEPARATOR_C);
AutoUTF			SlashDel(const AutoUTF &path);

bool			IsPathUnix(PCWSTR path);
inline bool		IsPathUnix(const AutoUTF &path) {
	return	IsPathUnix(path.c_str());
}

AutoUTF	ExtractFile(const AutoUTF &path);

AutoUTF	ExtractPath(const AutoUTF &path);

AutoUTF			GetSpecialPath(int csidl, bool create = true);

bool			MaskMatch(PCWSTR path, PCWSTR mask, DWORD flags = 0);

AutoUTF			MakePath(PCWSTR path, PCWSTR name);
inline AutoUTF	MakePath(const AutoUTF &path, const AutoUTF &name) {
	return	MakePath(path.c_str(), name.c_str());
}

AutoUTF			PathUnix(PCWSTR path);
inline AutoUTF	PathUnix(const AutoUTF &path) {
	return	PathUnix(path.c_str());
}

AutoUTF			PathWin(PCWSTR path);
inline AutoUTF	PathWin(const AutoUTF &path) {
	return	PathWin(path.c_str());
}

AutoUTF			GetWorkDirectory();

bool			SetWorkDirectory(PCWSTR path);
inline bool		SetWorkDirectory(const AutoUTF &path) {
	return	SetWorkDirectory(path.c_str());
}

AutoUTF			get_root(PCWSTR path);
inline AutoUTF	get_root(const AutoUTF &path) {
	return get_root(path.c_str());
}

bool			is_path_mask(PCWSTR path);
inline bool		is_path_mask(const AutoUTF &path) {
	return is_path_mask(path.c_str());
}

bool			is_valid_filename(PCWSTR name);
inline bool		is_valid_filename(const AutoUTF &name) {
	return	is_valid_filename(name.c_str());
}

AutoUTF			remove_path_prefix(const AutoUTF &path, PCWSTR pref = PATH_PREFIX_NT);

AutoUTF			ensure_path_prefix(const AutoUTF &path, PCWSTR pref = PATH_PREFIX_NT);

AutoUTF			get_path_from_mask(const AutoUTF &mask);

AutoUTF			TempDir();

AutoUTF			TempFile(PCWSTR path);
inline AutoUTF	TempFile(const AutoUTF &path) {
	return	TempFile(path.c_str());
}

bool substr_match(const AutoUTF& str, size_t pos, PCWSTR mstr);

void locate_path_root(const AutoUTF& path, size_t& path_root_len, bool& is_unc_path);

AutoUTF extract_file_name(const AutoUTF& path);

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

uint64_t	get_size(PCWSTR path);
inline uint64_t	get_size(const AutoUTF &path) {
	return get_size(path.c_str());
}

uint64_t	get_size(HANDLE hFile);

uint64_t	get_position(HANDLE hFile);

bool		set_position(HANDLE hFile, uint64_t pos, DWORD m = FILE_BEGIN);

//inline FILETIME		FileTimeCr(PCWSTR path) {
//	FileInfo	info;
//	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
//	return	info.ftCreationTime;
//}
//inline FILETIME		FileTimeAc(PCWSTR path) {
//	FileInfo	info;
//	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
//	return	info.ftLastAccessTime;
//}
//inline FILETIME		FileTimeWr(PCWSTR path) {
//	FileInfo	info;
//	::GetFileAttributesExW(path, GetFileExInfoStandard, &info);
//	return	info.ftLastWriteTime;
//}

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

inline bool is_dir_empty(PCWSTR path) {
	return	::PathIsDirectoryEmptyW(path);
}
inline bool is_dir_empty(const AutoUTF &path) {
	return	is_dir_empty(path.c_str());
}

bool create_directory(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr);
inline bool create_directory(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return create_directory(path.c_str(), lpsa);
}

bool create_directory_full(const AutoUTF &p, LPSECURITY_ATTRIBUTES sa = nullptr);

inline bool create_dir(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return	::SHCreateDirectoryExW(nullptr, path, lpsa) == ERROR_SUCCESS;
}
inline bool create_dir(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return create_dir(path.c_str(), lpsa);
}

bool create_file(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr);
inline bool create_file(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
	return create_file(path.c_str(), lpsa);
}

inline bool create_hardlink(PCWSTR path, PCWSTR newfile) {
	return	::CreateHardLinkW(newfile, path, nullptr) != 0;
}
inline bool create_hardlink(const AutoUTF &path, const AutoUTF &newfile) {
	return	create_hardlink(path.c_str(), newfile.c_str());
}

bool delete_dir(PCWSTR path);
inline bool delete_dir(const AutoUTF &path) {
	return	delete_dir(path.c_str());
}

bool delete_file(PCWSTR path);
inline bool delete_file(const AutoUTF &path) {
	return	delete_file(path.c_str());
}

bool delete_sh(PCWSTR path);
inline bool delete_sh(const AutoUTF &path) {
	return delete_sh(path.c_str());
}

inline bool delete_on_reboot(PCWSTR path) {
	return	::MoveFileExW(path, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
}
inline bool delete_on_reboot(const AutoUTF &path) {
	return	delete_on_reboot(path.c_str());
}

bool delete_recycle(PCWSTR path);
inline bool delete_recycle(const AutoUTF &path) {
	return	delete_recycle(path.c_str());
}

class DeleteFileCmd: public Command {
public:
	DeleteFileCmd(const AutoUTF &path):
		m_path(path) {
	}
	bool Execute() const {
		return delete_file(m_path) || delete_dir(m_path);
	}
private:
	AutoUTF m_path;
};

inline bool copy_file(PCWSTR path, PCWSTR dest) {
	return	::CopyFileW(path, dest, true) != 0;
}
inline bool copy_file(const AutoUTF &path, const AutoUTF &dest) {
	return	copy_file(path.c_str(), dest.c_str());
}

class CopyFileCmd: public Command {
public:
	CopyFileCmd(const AutoUTF &path, const AutoUTF &dest):
		m_path(path),
		m_dest(dest) {
	}
	bool Execute() const {
		return copy_file(m_path, m_dest);
	}
private:
	AutoUTF m_path, m_dest;
};

inline bool move_file(PCWSTR path, PCWSTR dest, DWORD flag = 0) {
	return	::MoveFileExW(path, dest, flag);
}
inline bool move_file(const AutoUTF &path, const AutoUTF &dest, DWORD flag = 0) {
	return	move_file(path.c_str(), dest.c_str(), flag);
}

class MoveFileCmd: public Command {
public:
	MoveFileCmd(const AutoUTF &path, const AutoUTF &dest):
		m_path(path),
		m_dest(dest) {
	}
	bool Execute() const {
		return move_file(m_path, m_dest);
	}
private:
	AutoUTF m_path, m_dest;
};

inline bool read_file(HANDLE hFile, PBYTE buf, DWORD &size) {
	return	::ReadFile(hFile, buf, size, &size, nullptr) != 0;
}
bool read_file(PCWSTR path, astring &buf);

bool get_file_inode(PCWSTR path, uint64_t &inode, size_t &nlink);

bool			FileCreate(PCWSTR path, PCWSTR name, PCSTR content);
inline bool		FileCreate(const AutoUTF &path, const AutoUTF &name, PCSTR content) {
	return	FileCreate(path.c_str(), name.c_str(), content);
}

bool			FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite = false);
inline bool		FileWrite(PCWSTR path, PCWSTR data, size_t size, bool rewrite = false) {
	return	FileWrite(path, data, size * sizeof(WCHAR), rewrite);
}
inline bool		FileWrite(const AutoUTF &path, const AutoUTF &data, bool rewrite = false) {
	return	FileWrite(path.c_str(), (PCVOID)data.c_str(), data.size() * sizeof(WCHAR), rewrite);
}
inline size_t	FileWrite(HANDLE file, const PVOID &in, size_t size) {
	DWORD	Result = 0;
	::WriteFile(file, in, size, &Result, nullptr);
	return	Result;
}
inline size_t	FileWrite(HANDLE file, const AutoUTF &in) {
	return	FileWrite(file, (const PVOID)in.c_str(), in.size() * sizeof(WCHAR));
}

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
	operator	FILETIME() const {
		return	*this;
	}
	operator	ULARGE_INTEGER() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = this->dwLowDateTime;
		Result.HighPart = this->dwHighDateTime;
		return	Result;
	}
	operator	uint64_t() const {
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
bool	WowDisable(PVOID &oldValue);
bool	WowEnable(PVOID &oldValue);
bool	IsWOW64();

///====================================================================================== WinSysInfo
struct		WinSysInfo: public SYSTEM_INFO {
	WinSysInfo();
	size_t			Uptime(size_t del = 1000);
};

#endif // WIN_DEF_HPP
