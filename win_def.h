/**
	win_def
	Main windows application include, always include first
	@classes	(WinMem, WinStr, CStr, WinFlag, WinBit, WinTimer, WinErrorCheck, Uncopyable, WinCOM)
	@author		© 2010 Andrew Grechkin
	@link		()
	@link		(ole32) for WinCom
**/

#ifndef WIN_DEF_HPP
#define WIN_DEF_HPP

#include <win_std.h>

#include <shlwapi.h>
#include <stdio.h>

#include <psapi.h>

extern "C" {
	WINBASEAPI ULONGLONG WINAPI	GetTickCount64();

	_CRTIMP int __cdecl			_snwprintf(wchar_t*, size_t, const wchar_t*, ...);
	long long __MINGW_NOTHROW	wcstoll(const wchar_t * __restrict__, wchar_t** __restrict__, int);
	unsigned long long __MINGW_NOTHROW wcstoull(const wchar_t * __restrict__, wchar_t ** __restrict__, int);
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
	Type	value;
	PCWSTR	name;

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

///=================================================================================== WinErrorCheck
/// Базовый класс для проверки и хранения кода ошибки
class		WinErrorCheck {
	DWORD	mutable	m_err;
protected:
	~WinErrorCheck() {
	}
	WinErrorCheck(): m_err(NO_ERROR) {
	}
public:
	DWORD			err() const {
		return	(DWORD)m_err;
	}
	DWORD			err(DWORD err) const {
		return	(DWORD)(m_err = err);
	}
	bool			IsOK() const {
		return	m_err == NO_ERROR;
	}
	bool			ChkSucc(bool in) const {
		if (!in) {
			err(::GetLastError());
		} else {
			err(NO_ERROR);
		}
		return	in;
	}
	template<typename Type>
	void			SetIfFail(Type &in, const Type &value) {
		if (m_err != NO_ERROR) {
			in = value;
		}
	}
};

///=================================================================================================
template<typename Type>
inline Type		ReverseBytes(const Type &in) {
	Type	Result;
	size_t	size = sizeof(Type) - 1;
	char	*sta = (char*)(&in), *end = (char*)(&Result);
	for (size_t i = 0; i <= size; ++i) {
		end[size-i] = sta[i];
	}
	return	(Result);
}

///====================================================================================== Shared_ptr
template <typename Type>
class	Shared_ptr {
	template <typename Pti>
	class	Pointee {
		size_t	m_ref;
		Pti		*m_ptr;
	public:
		Pointee(Pti *ptr): m_ref(1), m_ptr(ptr) {
		}
		void	delRef() {
			--m_ref;
			if (m_ref == 0 && m_ptr)
				delete	m_ptr;
		}
		void	addRef() {
			++m_ref;
		}
		friend class Shared_ptr;
	};
	Pointee<Type>	*data;
public:
	~Shared_ptr() {
		release();
	}
	Shared_ptr(): data(new Pointee<Type>(nullptr)) {
	}
	Shared_ptr(Type *ptr): data(new Pointee<Type>(ptr)) {
	}
	Shared_ptr(const Shared_ptr<Type> &rhs): data(rhs.data) {
		data->addRef();
	}

	Shared_ptr<Type>&	operator=(const Shared_ptr<Type> &rhs) {
		if (this != &rhs) {
			release();
			data = rhs.data;
			data->addRef();
		}
		return	*this;
	}
	Type*			get() const {
		return	data->m_ptr;
	}
	void			release() {
		data->delRef();
	}
	void			reset(Type* ptr) {
		release();
		data = new Pointee<Type>(ptr);
	}
	void			swap(Shared_ptr<Type> &rhs) {
		std::swap(data, rhs.data);
	}

	operator		bool() const {
		return	data->m_ptr;
	}
	operator		Type*() const {
		return	data->m_ptr;
	}
	Type*			operator->() const {
		return data->m_ptr;
	}
	Type&			operator*() {
		return *(data->m_ptr);
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
		explicit	MzsData(PCWSTR in) {
			PCWSTR	ptr = in;
			while (*ptr) {
				ptr += (Len(ptr) + 1);
				++m_size;
			}
			m_capa = ptr - in + 1;
			m_data = new WCHAR[m_capa];
			WinMem::Copy(m_data, in, m_capa * sizeof(WCHAR));
		}
		friend class CStrMW;
	};
	Shared_ptr<MzsData>	m_str;
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
		ssize_t	bit = 0;
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
int					consoleout(PCSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE);
inline int			consoleout(PCWSTR in, DWORD nStdHandle = STD_OUTPUT_HANDLE) {
	HANDLE hStdOut = ::GetStdHandle(nStdHandle);
	if (hStdOut && hStdOut != INVALID_HANDLE_VALUE) {
		DWORD written = 0;
		DWORD len = Len(in);
		if (len && !::WriteConsoleW(hStdOut, in, len, &written, nullptr)) {
			::WriteFile(hStdOut, in, len * sizeof(*in), &written, nullptr);
			written /= sizeof(*in);
		}
		return written;
	}
	return 0;
}
inline int			consoleout(WCHAR in, DWORD nStdHandle = STD_OUTPUT_HANDLE) {
	WCHAR out[] = {in, 0};
	return consoleout(out, nStdHandle);
}
inline int			consoleout(const AutoUTF &in, DWORD nStdHandle = STD_OUTPUT_HANDLE/*STD_ERROR_HANDLE*/) {
	return consoleout(in.c_str(), nStdHandle);
}

inline int vsnprintf(PWSTR buf, size_t len, PCWSTR format, va_list vl) {
	buf[len - 1] = 0;
	return ::_vsnwprintf(buf, len - 1, format, vl);
}
inline int stdvprintf(DWORD nStdHandle, PCWSTR format, va_list vl) {
	WCHAR buff[8 * 1024];
	vsnprintf(buff, sizeofa(buff), format, vl);
	return consoleout(buff, nStdHandle);
}
int			stdprintf(DWORD nStdHandle, PCWSTR format, ...);

inline int 	printf(PCWSTR format, ...) {
	va_list vl;
	va_start(vl, format);
	int Result = stdvprintf(STD_OUTPUT_HANDLE, format, vl);
	va_end(vl);
	return Result;
}

inline int			vprintf(PCWSTR format, va_list vl) {
	return	stdvprintf(STD_OUTPUT_HANDLE, format, vl);
}
int					snprintf(PWSTR buff, size_t len, PCWSTR format, ...);
void				errx(int eval, PCSTR format, ...);

enum		WinLogLevel {
	LOG_TRACE =	-3,
	LOG_DEBUG,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_ERROR,
};

extern int			logLevel;
void				setLogLevel(WinLogLevel lvl);
void				logError(PCWSTR format, ...);
void				logError(DWORD errNumber, PCWSTR format, ...);
void				logDebug(PCWSTR format, ...);
void				logVerbose(PCWSTR format, ...);
void				logCounter(PCWSTR format, ...);
void				logInfo(PCWSTR format, ...);
void				logFile(WIN32_FIND_DATA FileData);

///========================================================================================== WinEnv
namespace	WinEnv {
	AutoUTF	Get(PCWSTR name);
	bool	Set(PCWSTR name, PCWSTR val);
	bool	Add(PCWSTR name, PCWSTR val);
	bool	Del(PCWSTR name);
}

inline DWORD		UserLogon(HANDLE &hToken, PCWSTR name, PCWSTR pass, DWORD type, PCWSTR dom = L"") {
	DWORD	Result = NO_ERROR;
	if (!::LogonUserW((PWSTR)name, (PWSTR)dom, (PWSTR)pass, type, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		Result = ::GetLastError();
	}
	return	Result;
}

///============================================================================================ path
inline AutoUTF	Canonicalize(PCWSTR path) {
	WCHAR	Result[MAX_PATH_LEN];
	::PathCanonicalizeW(Result, path);
	return	Result;
}
inline AutoUTF	Canonicalize(const AutoUTF &path) {
	return	Canonicalize(path.c_str());
}
inline AutoUTF	Expand(PCWSTR path) {
	DWORD	size = ::ExpandEnvironmentStringsW(path, nullptr, 0);
	if (size) {
		WCHAR	Result[::ExpandEnvironmentStringsW(path, nullptr, 0)];
		if (::ExpandEnvironmentStringsW(path, Result, size))
			return	Result;
	}
	return	AutoUTF();
}
inline AutoUTF	Expand(const AutoUTF &path) {
	return	Expand(path.c_str());
}
inline AutoUTF	PathNice(PCWSTR path) {
	return	Canonicalize(Expand(path));
}
inline AutoUTF	PathNice(const AutoUTF &path) {
	return	PathNice(path.c_str());
}
inline AutoUTF	PathCompact(PCWSTR path, size_t size) {
	WCHAR	Result[MAX_PATH_LEN];
	::PathCompactPathExW(Result, path, size, 0);
	return	Result;
}
inline AutoUTF	PathCompact(const AutoUTF &path, size_t size) {
	return	PathCompact(path.c_str(), size);
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
	return	SlashAdd(path.c_str());
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
	Result += name;
	return	Result;
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
	WCHAR	buf[::GetTempPathW(0, nullptr) + 1];
	buf[0] = 0;
	::GetTempPathW(sizeofa(buf), buf);
	return	buf;
}
inline AutoUTF		TempFile(PCWSTR path) {
	WCHAR	buf[MAX_PATH];
	WCHAR	pid[32];
	Num2Str(pid, ::GetCurrentProcessId());
	buf[0] = 0;
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

bool file_exists(PCWSTR path);
inline bool file_exists(const AutoUTF &path) {
	return	file_exists(path.c_str());
}

bool is_file(PCWSTR path);
inline bool is_file(const AutoUTF &path) {
	return	is_file(path.c_str());
}

bool is_dir(PCWSTR path);
inline bool is_dir(const AutoUTF &path) {
	return	is_dir(path.c_str());
}

bool dir_is_empty(PCWSTR path);
inline bool dir_is_empty(const AutoUTF &path) {
	return	dir_is_empty(path.c_str());
}

bool create_dir(PCWSTR path, LPSECURITY_ATTRIBUTES lpsa = nullptr);
inline bool create_dir(const AutoUTF &path, LPSECURITY_ATTRIBUTES lpsa = nullptr) {
    return create_dir(path.c_str(), lpsa);
}

inline bool create_hardlink(PCWSTR path, PCWSTR newfile) {
	return	::CreateHardLinkW(newfile, path, nullptr) != 0;
}
inline bool create_hardlink(const AutoUTF &path, const AutoUTF &newfile) {
	return	create_hardlink(path.c_str(), newfile.c_str());
}

bool ensure_dir_exist(PCWSTR path);
inline bool ensure_dir_exist(const AutoUTF &path) {
	return	ensure_dir_exist(path.c_str());
}

bool is_path_mask(PCWSTR path);
inline bool is_path_mask(const AutoUTF &path) {
	return is_path_mask(path.c_str());
}

AutoUTF remove_path_prefix(const AutoUTF &path, PCWSTR pref = PATH_PREFIX_NT);

AutoUTF ensure_path_prefix(const AutoUTF &path, PCWSTR pref = PATH_PREFIX_NT);

AutoUTF	get_path_from_mask(const AutoUTF &mask);

bool delete_dir(PCWSTR path);
inline bool delete_dir(const AutoUTF &path) {
	return	delete_dir(path.c_str());
}

bool delete_file(PCWSTR path);
inline bool delete_file(const AutoUTF &path) {
	return	delete_file(path.c_str());
}

bool delete_sh(PCWSTR path);

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
inline bool			FileCreate(const AutoUTF &path, const AutoUTF &name, PCSTR content) {
	return	FileCreate(path.c_str(), name.c_str(), content);
}

bool				FileWrite(PCWSTR path, PCVOID buf, size_t size, bool rewrite = false);
inline bool			FileWrite(PCWSTR path, PCWSTR data, size_t size, bool rewrite = false) {
	return	FileWrite(path, data, size * sizeof(WCHAR), rewrite);
}
inline bool			FileWrite(const AutoUTF &path, const AutoUTF &data, bool rewrite = false) {
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
bool				FileWipe(PCWSTR path);

AutoUTF				GetDrives();

///========================================================================================= WinFile
class		WinFile: private Uncopyable, public WinErrorCheck {
	HANDLE	m_hndl;
public:
	~WinFile() {
		Close();
	}
	WinFile(): m_hndl(INVALID_HANDLE_VALUE) {
	}
	WinFile(PCWSTR path, bool write = false): m_hndl(INVALID_HANDLE_VALUE) {
		Open(path, write);
	}
	operator		HANDLE() const {
		return	m_hndl;
	}

	bool			Open(PCWSTR path, ACCESS_MASK access, DWORD share, PSECURITY_ATTRIBUTES sa, DWORD creat, DWORD flags) {
		Close();
		m_hndl = ::CreateFileW(path, access, share, sa, creat, flags, nullptr);
		return	m_hndl && m_hndl != INVALID_HANDLE_VALUE;
	}
	bool			Open(PCWSTR path, bool write = false) {
		ACCESS_MASK	amask = (write) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
		DWORD		share = (write) ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
		DWORD		creat = (write) ? OPEN_EXISTING : OPEN_EXISTING;
		DWORD		flags = (write) ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
		return	Open(path, amask, share, nullptr, creat, flags);
	}
	void			Close() {
		::CloseHandle(m_hndl);
		m_hndl = INVALID_HANDLE_VALUE;
	}
	bool			Attr(DWORD attr) {
		WCHAR	path[MAX_PATH_LEN];
		Path(path, sizeofa(path));
		return	::SetFileAttributesW(path, attr);
	}
	DWORD			Attr() const {
		WCHAR	path[MAX_PATH_LEN];
		Path(path, sizeofa(path));
		return	::GetFileAttributesW(path);
	}
	bool			Size(uint64_t &size) {
		LARGE_INTEGER	tmp;
		if (::GetFileSizeEx(m_hndl, &tmp)) {
			size = tmp.QuadPart;
			return	true;
		}
		return	false;
	}
	bool			Path(PWSTR path, size_t len) const;

	bool			Write(PVOID buf, size_t size, DWORD &written) {
		return	::WriteFile(m_hndl, (PCSTR)buf, size, &written, nullptr);
	}
	bool			Pointer(uint64_t dist, DWORD moveMethod) {
		LARGE_INTEGER	tmp;
		tmp.QuadPart = dist;
		return	::SetFilePointerEx(m_hndl, tmp, nullptr, moveMethod);
	}
	bool			SetEnd() {
		return	::SetEndOfFile(m_hndl);
	}
};

///======================================================================================= WinFileId
class		WinFileId {
	DWORD	m_vol_sn;
	DWORD	m_node_low;
	DWORD	m_node_high;
	DWORD	m_links;
public:
	WinFileId(): m_vol_sn(0), m_node_low(0), m_node_high(0), m_links(0)  {
	}
	WinFileId(const BY_HANDLE_FILE_INFORMATION &info):
			m_vol_sn(info.dwVolumeSerialNumber),
			m_node_low(info.nFileIndexLow),
			m_node_high(info.nFileIndexHigh),
			m_links(info.nNumberOfLinks)  {

	}
	WinFileId(HANDLE hFile): m_vol_sn(0), m_node_low(0), m_node_high(0), m_links(0) {
		Load(hFile);
	}
	WinFileId(PCWSTR path): m_vol_sn(0), m_node_low(0), m_node_high(0), m_links(0)  {
		HANDLE	hFile = ::CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
									 FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		Load(hFile);
		::CloseHandle(hFile);
	}

	bool			Load(PCWSTR path) {
		HANDLE	hFile;
		if (FileOpenAttr(path, hFile)) {
			bool	Result = Load(hFile);
			::CloseHandle(hFile);
			return	Result;
		}
		return	false;
	}
	bool			Load(HANDLE hFile) {
		if (!m_links) {
			BY_HANDLE_FILE_INFORMATION	info;
			if (::GetFileInformationByHandle(hFile, &info) && (info.dwVolumeSerialNumber != 0 || info.nFileIndexLow != 0 || info.nFileIndexHigh != 0)) {
				m_vol_sn = info.dwVolumeSerialNumber;
				m_node_low = info.nFileIndexLow;
				m_node_high = info.nFileIndexHigh;
				m_links = info.nNumberOfLinks;
				return	true;
			}
		}
		return	false;
	}

	bool			operator==(const WinFileId &in) const {
		return	(m_vol_sn == in.m_vol_sn) && (m_node_low == in.m_node_low) && (m_node_high == in.m_node_high);
	}
	bool			operator!=(const WinFileId &in) const {
		return	!operator==(in);
	}
	bool			IsOK() {
		return	m_links;
	}

	DWORD			vol_sn() const {
		return	m_vol_sn;
	}
	DWORD			links() const {
		return	m_links;
	}
	DWORD			node_low() const {
		return	m_node_low;
	}
	DWORD			node_high() const {
		return	m_node_high;
	}
	uint64_t		node() const {
		ULARGE_INTEGER	Result;
		Result.LowPart = m_node_low;
		Result.HighPart = m_node_high;
		Result.QuadPart &= 0x0000FFFFFFFFFFFFULL;
		return	Result.QuadPart;
	}
};

///========================================================================================= FileMap
/// Отображение файла в память блоками
class		FileMap : private Uncopyable, public WinErrorCheck {
	HANDLE		m_hSect;
	PVOID		m_data;
	size_t		m_framesize;
	uint64_t	m_mapsize;
	uint64_t	m_offset;
	bool		m_write;
public:
	~FileMap() {
		Close();
	}
	FileMap(const WinFile &wf, uint64_t size = (uint64_t) - 1, bool write = false): m_hSect(nullptr), m_data(nullptr), m_mapsize(0), m_offset(0) {
		Open(wf, size, write);
	}
	FileMap(PCWSTR path, uint64_t size = (uint64_t) - 1, bool write = false): m_hSect(nullptr), m_data(nullptr), m_mapsize(0), m_offset(0) {
		Open(path, size, write);
	}

	bool			Close() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = nullptr;
		}
		if (m_hSect) {
			::CloseHandle(m_hSect);
			m_hSect = nullptr;
		}
		return	true;
	}
	bool			Open(const WinFile &wf, uint64_t size = (uint64_t) - 1, bool write = false) {
		Close();
		Home();
		HANDLE	hFile = wf;
		m_write = write;
		if (hFile && hFile != INVALID_HANDLE_VALUE) {
			DWORD	protect = (m_write) ? PAGE_READWRITE : PAGE_READONLY;
			m_mapsize = std::min(FileSize(hFile), size);
			m_hSect = ::CreateFileMapping(hFile, nullptr, protect, (DWORD)(m_mapsize >> 32), (DWORD)(m_mapsize & 0xFFFFFFFF), nullptr);
		}
		return	ChkSucc(m_hSect != nullptr);
	}
	bool			Open(PCWSTR path, uint64_t size = (uint64_t) - 1, bool write = false) {
		WinFile	wf(path, write);
		return	Open(wf, size, write);
	}

	bool			Next() {
		if (m_data) {
			::UnmapViewOfFile(m_data);
			m_data = nullptr;
		}
		if ((m_mapsize - m_offset) > 0) {
			if ((m_mapsize - m_offset) < (uint64_t)m_framesize)
				m_framesize = (size_t)(m_mapsize - m_offset);
			if (m_hSect) {
				ACCESS_MASK	amask = (m_write) ? FILE_MAP_WRITE : FILE_MAP_READ;
				m_data = ::MapViewOfFile(m_hSect, amask, (DWORD)(m_offset >> 32), (DWORD)(m_offset & 0xFFFFFFFF), m_framesize);
				m_offset += m_framesize;
				return	ChkSucc(m_data != nullptr);
			}
		}
		return	false;
	}
	bool			Home() {
		SYSTEM_INFO	info;
		::GetSystemInfo(&info);
		m_framesize = info.dwAllocationGranularity * 128;
		m_offset = 0LL;
		return	true;
	}

	PVOID			data() const {
		return	m_data;
	}
	size_t			size() const {
		return	m_framesize;
	}
	uint64_t		offset() const {
		return	m_offset;
	}
	uint64_t		sizeOfMap() const {
		return	m_mapsize;
	}
};

///========================================================================================== WinVol
class		WinVol : private Uncopyable, public WinErrorCheck {
	HANDLE		m_hnd;
	AutoUTF		name;

	void					Close();
public:
	~WinVol() {
		Close();
	}
	WinVol(): m_hnd(INVALID_HANDLE_VALUE) {}
	bool 					Next();

	AutoUTF			GetName() const {
		return	name;
	}
	AutoUTF			GetPath() const;
	AutoUTF			GetDevice() const;

	long long		GetSize() const {
//		long long tmp = f_.nFileSizeHigh;
//		tmp = tmp << (sizeof(f_.nFileSizeHigh) * 8);
//		tmp |= f_.nFileSizeLow;
		return	0;
	}

	DWORD			GetFlag() const {
		DWORD	Result = 0;
//		::GetVolumeInformation(path.c_str(), nullptr, 0, nullptr, nullptr, &Result, nullptr, 0);
		return	Result;
	}
	UINT			GetType() const {
		return	::GetDriveTypeW(name.c_str());
	}

	bool			IsSuppCompress() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_FILE_COMPRESSION);
	}
	bool			IsSuppEncrypt() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_SUPPORTS_ENCRYPTION);
	}
	bool			IsSuppStreams() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_NAMED_STREAMS);
	}
	bool			IsSuppACL() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_PERSISTENT_ACLS);
	}
	bool			IsReadOnly() const {
		return	WinFlag::Check(GetFlag(), (DWORD)FILE_READ_ONLY_VOLUME);
	}

	bool			IsRemovable() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_REMOVABLE);
	}
	bool			IsFixed() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_FIXED);
	}
	bool			IsRemote() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_REMOTE);
	}
	bool			IsCdRom() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_CDROM);
	}
	bool			IsRamdisk() const {
		return	WinFlag::Check(GetType(), (UINT)DRIVE_RAMDISK);
	}

	bool			GetSize(uint64_t &uiUserFree, uint64_t &uiTotalSize, uint64_t &uiTotalFree) const;
	uint64_t		GetSizeTotal() const {
		uint64_t uf = 0, ts = 0, tf = 0;
		GetSize(uf, ts, tf);
		return	ts;
	}
};

///========================================================================================== WinDir
class		WinDir : private Uncopyable, public WinErrorCheck {
	WIN32_FIND_DATAW	m_find;
	HANDLE				m_handle;
	AutoUTF				m_path;
	AutoUTF				m_mask;

	void			Close() {
		if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
			::FindClose(m_handle);
			m_handle = nullptr;
		}
	}
public:
	~WinDir() {
		Close();
	}
	WinDir(const AutoUTF &path, const AutoUTF &mask = L"*"): m_handle(nullptr), m_path(path), m_mask(mask) {
	}

	bool 			Next() {
		AutoUTF	tmp(SlashAdd(m_path));
		tmp += m_mask;
		if (m_handle == nullptr) {
			m_handle = ::FindFirstFileW(tmp.c_str(), &m_find);
			ChkSucc(m_handle != INVALID_HANDLE_VALUE);
		} else {
			ChkSucc(::FindNextFileW(m_handle, &m_find) != 0);
		}
		if (IsOK() && !is_valid_filename(m_find.cFileName))
			return	Next();
		return	IsOK();
	}

	void			path(const AutoUTF &in) {
		Close();
		m_path = in;
	}
	void			mask(const AutoUTF &in) {
		Close();
		m_mask = in;
	}

	bool			IsDir() const {
		return	m_find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}
	bool			IsJunc() const {
		return	m_find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
	}
	DWORD			attr() const {
		return	m_find.dwFileAttributes;
	}
	FILETIME		time_cr() const {
		return	m_find.ftCreationTime;
	}
	FILETIME		time_ac() const {
		return	m_find.ftLastAccessTime;
	}
	FILETIME		time_wr() const {
		return	m_find.ftLastWriteTime;
	}
	PCWSTR			name() const {
		return	m_find.cFileName;
	}
	AutoUTF			name_full() const {
		AutoUTF	Result(SlashAdd(m_path));
		Result += m_find.cFileName;
		return	Result;
	}
	PCWSTR			name_dos() const {
		return	m_find.cAlternateFileName;
	}
	PCWSTR			path() const {
		return	m_path.c_str();
	}
	uint64_t		size() const {
		return	MyUI64(m_find.nFileSizeLow, m_find.nFileSizeHigh);
	}
};

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

///========================================================================================== WinReg
class		WinReg: private Uncopyable {
	HKEY	mutable	hKeyOpend;
	HKEY			hKeyReq;
	AutoUTF			m_path;

	void			CloseKey() const;
	bool			OpenKey(ACCESS_MASK acc) const {
		return	OpenKey(hKeyReq, m_path, acc);
	}
	bool			OpenKey(HKEY hkey, const AutoUTF &path, ACCESS_MASK acc) const;

	template <typename Type>
	void			SetRaw(const AutoUTF &name, const Type &value, DWORD type = REG_BINARY) const {
		if (OpenKey(KEY_WRITE)) {
			::RegSetValueExW(hKeyOpend, name.c_str(), 0, type, (PBYTE)(&value), sizeof(value));
			CloseKey();
		}
	}
	template <typename Type>
	bool			GetRaw(const AutoUTF &name, Type &value, const Type &def) const {
		bool	Result = OpenKey(KEY_READ);
		value = def;
		if (Result) {
			DWORD	size = sizeof(value);
			Result = ::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, nullptr, (PBYTE)(&value), &size) == ERROR_SUCCESS;
			CloseKey();
		}
		return	Result;
	}
public:
	~WinReg() {
		CloseKey();
	}
	WinReg(): hKeyOpend(0), hKeyReq(HKEY_CURRENT_USER) {
	}
	WinReg(HKEY hkey, const AutoUTF &path): hKeyOpend(0), hKeyReq(hkey), m_path(path) {
	}
	WinReg(const AutoUTF &path);

	AutoUTF			path() const {
		return	m_path;
	}
	void			path(const AutoUTF &path) {
		m_path = path;
	}
	void			key(HKEY hkey) {
		hKeyReq = hkey;
	}

	bool			Add(const AutoUTF &name) const;
	bool			Del(const AutoUTF &name) const;

	void			Set(const AutoUTF &name, PCWSTR value) const;
	void			Set(const AutoUTF &name, const AutoUTF &value) const {
		Set(name, value.c_str());
	}
	void			Set(const AutoUTF &name, int value) const;

	bool			Get(const AutoUTF &name, AutoUTF &value, const AutoUTF &def) const;
	bool			Get(const AutoUTF &name, int &value, int def) const;
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

///========================================================================================= WinPerf
struct		WinPerf: public PERFORMANCE_INFORMATION {
	WinPerf();
};

#endif // WIN_DEF_HPP
