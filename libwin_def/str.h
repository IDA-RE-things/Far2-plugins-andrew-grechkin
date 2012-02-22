#ifndef _WIN_DEF_STR_HPP
#define _WIN_DEF_STR_HPP

#include "std.h"
#include "bit.h"
#include "memory.h"

///====================================================================== Функции работы со строками
inline size_t Len(PCSTR in) {
	return ::strlen(in);
}

inline size_t Len(PCWSTR in) {
	return ::wcslen(in);
}

inline bool Empty(PCSTR in) {
	return in[0] == 0;
}

inline bool Empty(PCWSTR in) {
	return in[0] == 0;
}

#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif

inline int CmpCode(PCSTR in1, PCSTR in2) {
	return ::strcmp(in1, in2);
}

inline int CmpCode(PCSTR in1, PCSTR in2, size_t n) {
	return ::strncmp(in1, in2, n);
}

inline int CmpCode(PCWSTR in1, PCWSTR in2) {
	return ::wcscmp(in1, in2);
	//	return ::wcscoll(in1, in2);
}

inline int CmpCode(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::wcsncmp(in1, in2, n);
}

inline int Cmp(PCSTR in1, PCSTR in2) {
	return ::CompareStringA(0, SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmp(PCSTR in1, PCSTR in2, size_t n) {
	return ::CompareStringA(0, NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}

inline int Cmp(PCWSTR in1, PCWSTR in2) {
	return ::CompareStringW(0 , SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmp(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::CompareStringW(0 , NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2, n) - CSTR_EQUAL;
}

inline int Cmpi(PCSTR in1, PCSTR in2) {
	//	return ::_stricmp(in1, in2);
	return ::CompareStringA(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmpi(PCSTR in1, PCSTR in2, size_t n) {
	return ::CompareStringA(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2,
	                        n) - CSTR_EQUAL;
}

inline int Cmpi(PCWSTR in1, PCWSTR in2) {
	//	return ::_wcsicmp(in1, in2);
	//	return ::_wcsicoll(lhs.first.c_str(), rhs.first.c_str()) < 0;
	//	return fsf.LStricmp(lhs.first.c_str(), rhs.first.c_str()) < 0;
	return ::CompareStringW(0, NORM_IGNORECASE | SORT_STRINGSORT, in1, -1, in2, -1) - CSTR_EQUAL;
}

inline int Cmpi(PCWSTR in1, PCWSTR in2, size_t n) {
	return ::CompareStringW(0, NORM_IGNORECASE | NORM_STOP_ON_NULL | SORT_STRINGSORT, in1, n, in2,
	                        n) - CSTR_EQUAL;
}

inline bool EqCode(PCSTR in1, PCSTR in2) {
	return CmpCode(in1, in2) == 0;
}

inline bool EqCode(PCWSTR in1, PCWSTR in2) {
	return CmpCode(in1, in2) == 0;
}

inline bool Eq(PCSTR in1, PCSTR in2) {
	return Cmp(in1, in2) == 0;
}

inline bool Eq(PCWSTR in1, PCWSTR in2) {
	return Cmp(in1, in2) == 0;
}

inline bool Eqi(PCSTR in1, PCSTR in2) {
	return Cmpi(in1, in2) == 0;
}

inline bool Eqi(PCWSTR in1, PCWSTR in2) {
	return Cmpi(in1, in2) == 0;
}

inline PSTR Copy(PSTR dest, PCSTR src) {
	return ::strcpy(dest, src);
}

inline PWSTR Copy(PWSTR dest, PCWSTR src) {
	return ::wcscpy(dest, src);
}

inline PSTR Copy(PSTR dest, PCSTR src, size_t size) {
	return ::strncpy(dest, src, size);
}

inline PWSTR Copy(PWSTR dest, PCWSTR src, size_t size) {
	return ::wcsncpy(dest, src, size);
}

inline PSTR Cat(PSTR dest, PCSTR src) {
	return ::strcat(dest, src);
}

inline PWSTR Cat(PWSTR dest, PCWSTR src) {
	return ::wcscat(dest, src);
}

inline PSTR Cat(PSTR dest, PCSTR src, size_t size) {
	return ::strncat(dest, src, size);
}

inline PWSTR Cat(PWSTR dest, PCWSTR src, size_t size) {
	return ::wcsncat(dest, src, size);
}

inline PSTR Find(PCSTR where, PCSTR what) {
	return ::strstr(where, what);
}

inline PCSTR Find(PCSTR where, CHAR what) {
	return ::strchr(where, what);
}

inline PWSTR Find(PCWSTR where, PCWSTR what) {
	return ::wcsstr(where, what);
}

inline PCWSTR Find(PCWSTR where, WCHAR what) {
	return ::wcschr(where, what);
}

inline PSTR RFind(PCSTR where, PCSTR what) {
	PCSTR last1 = where + Len(where);
	PCSTR last2 = what + Len(what);
	last2 = std::find_end(where, last1, what, last2);
	return (last1 == last2) ? nullptr : const_cast<PSTR>(last2);
}

inline PSTR RFind(PCSTR where, CHAR what) {
	return ::strrchr(where, what);
}

inline PWSTR RFind(PCWSTR where, PCWSTR what) {
	PCWSTR last1 = where + Len(where);
	PCWSTR last2 = what + Len(what);
	last2 = std::find_end(where, last1, what, last2);
	return (last1 == last2) ? nullptr : const_cast<PWSTR>(last2);
}

inline PWSTR RFind(PCWSTR where, WCHAR what) {
	return ::wcsrchr(where, what);
}

inline size_t Span(PCSTR str, PCSTR strCharSet) {
	return ::strcspn(str, strCharSet);
}

inline size_t Span(PCWSTR str, PCWSTR strCharSet) {
	return ::wcscspn(str, strCharSet);
}

///=================================================================================================
inline PWSTR ToUpper(PWSTR buf, size_t len) {
	::CharUpperBuffW(buf, len);
	return buf;
}

inline PWSTR ToLower(PWSTR buf, size_t len) {
	::CharLowerBuffW(buf, len);
	return buf;
}

inline PWSTR ToUpper(PWSTR s1) {
	return ToUpper(s1, Len(s1));
}

inline PWSTR ToLower(PWSTR s1) {
	return ToLower(s1, Len(s1));
}

inline PSTR Fill(PSTR in, CHAR ch) {
	return ::_strset(in, ch);
}

inline PWSTR Fill(PWSTR in, WCHAR ch) {
	return ::_wcsset(in, ch);
}

inline PSTR Reverse(PSTR in) {
	return ::_strrev(in);
}

inline PWSTR Reverse(PWSTR in) {
	return ::_wcsrev(in);
}

//inline PWSTR AssignStr(PCWSTR src) {
//	size_t len = Len(src) + 1;
//	PWSTR dest;
//	WinMem::Alloc(dest, len * sizeof(WCHAR));
//	Copy(dest, src, len);
//	return dest;
//}

inline size_t Convert(PCSTR from, UINT cp, PWSTR to = nullptr, size_t size = 0) {
	return ::MultiByteToWideChar(cp, 0, from, -1, to, (int)size);
}

inline size_t Convert(PCWSTR from, UINT cp, PSTR to = nullptr, size_t size = 0) {
	return ::WideCharToMultiByte(cp, 0, from, -1, to, (int)size, nullptr, nullptr);
}

#ifdef NoStlString
#include "autostr.h"
#else
#include "autoutf.h"
#endif

inline size_t Len(const astring &in) {
	return in.size();
}
inline size_t Len(const ustring &in) {
	return in.size();
}

inline astring as_str_a(int64_t num, int base = 10) {
	CHAR buf[64];
	::_i64toa(num, buf, base);
	return astring(buf);
}

inline ustring as_str(int64_t num, int base = 10) {
	WCHAR buf[64];
	::_i64tow(num, buf, base);
	return ustring(buf);
}

inline astring oem(PCWSTR in) {
	return w2cp(in, CP_OEMCP);
}
inline astring oem(const ustring &in) {
	return w2cp(in.c_str(), CP_OEMCP);
}

inline astring ansi(PCWSTR in) {
	return w2cp(in, CP_ACP);
}
inline astring ansi(const ustring &in) {
	return w2cp(in.c_str(), CP_ACP);
}

inline astring utf8(PCWSTR in) {
	return w2cp(in, CP_UTF8);
}
inline astring utf8(const ustring &in) {
	return w2cp(in.c_str(), CP_UTF8);
}

inline ustring utf16(PCSTR in, UINT cp = CP_UTF8) {
	return cp2w(in, cp);
}
inline ustring utf16(const astring &in, UINT cp = CP_UTF8) {
	return cp2w(in.c_str(), cp);
}

astring& Trim_l(astring &str, const astring &chrs = " \t\r\n");

astring& Trim_r(astring &str, const astring &chrs = " \t\r\n");

astring& Trim(astring &str, const astring &chrs = " \t\r\n");

astring TrimOut(const astring &str, const astring &chrs = " \t\r\n");

ustring& Trim_l(ustring &str, const ustring &chrs = L" \t\r\n");

ustring& Trim_r(ustring &str, const ustring &chrs = L" \t\r\n");

ustring& Trim(ustring &str, const ustring &chrs = L" \t\r\n");

ustring TrimOut(const ustring &str, const ustring &chrs = L" \t\r\n");

ustring GetWord(const ustring &str, WCHAR d = PATH_SEPARATOR_C);

astring& AddWord(astring &inout, const astring &add, const astring &delim = "");

ustring& AddWord(ustring &inout, const ustring &add, const ustring &delim = L"");

astring& AddWordEx(astring &inout, const astring &add, const astring &delim = "");

ustring& AddWordEx(ustring &inout, const ustring &add, const ustring &delim = L"");

astring CutWord(astring &inout, const astring &delim = "\t ", bool delDelim = true);

ustring CutWord(ustring & inout, const ustring &delim = L"\t ", bool delDelim = true);

astring CutWordEx(astring & inout, const astring & delim, bool delDelim = true);

ustring CutWordEx(ustring & inout, const ustring & delim, bool delDelim = true);

ustring & ReplaceAll(ustring & str, const ustring & from, const ustring & to);

ustring ReplaceAllOut(const ustring& str, const ustring &from, const ustring &to);

inline void mbox(PCSTR text, PCSTR capt = "") {
	::MessageBoxA(nullptr, text, capt, MB_OK);
}

inline void mbox(PCWSTR text, PCWSTR capt = L"") {
	::MessageBoxW(nullptr, text, capt, MB_OK);
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ err
ustring ErrAsStr(DWORD err = ::GetLastError(), PCWSTR lib = nullptr);

inline ustring ErrWmiAsStr(HRESULT err) {
	return ErrAsStr(err, L"wmiutils.dll");
}

//template<typename Type>
//void StrToCont(const ustring &src, Type dst, const ustring &delim = L" \t\n\r") {
//	ustring::size_type start, end = 0;
//	while ((start = src.find_first_not_of(delim, end)) != ustring::npos) {
//		end = src.find_first_of(delim, start);
//		dst = src.substr(start, end - start);
//	}
//}

//#include <tr1/functional>
//using std::tr1::placeholders::_1;
//using std::tr1::placeholders::_2;
//
//template<typename Type>
//const Type * find_first_of(const Type * where, const Type * what) {
//	//	return (PWSTR)(in + ::wcscspn(in, mask));
//	using namespace std;
//	typedef const Type * str_t;
//	typedef str_t (*func_t)(str_t, Type);
//	str_t last1 = &where[Len(where)];
//	str_t pos = find_if(&where[0], last1, tr1::bind((func_t)Find, what, _1));
//	return (last1 == pos) ? nullptr : pos;
//}
//
//template<typename Type>
//const Type * find_first_not_of(const Type * where, const Type * what) {
//	//	return (PWSTR)(in + ::wcsspn(in, mask));
//	using namespace std;
//	typedef const Type * str_t;
//	typedef str_t (*func_t)(str_t, Type);
//	str_t last1 = &where[Len(where)];
//	str_t pos = find_if(&where[0], last1, tr1::bind(logical_not<bool>(), tr1::bind((func_t)Find, what, _1)));
//	return (last1 == pos) ? nullptr : pos;
//}
//
//template<typename Type>
//const Type * find_last_of(const Type * where, const Type * what) {
//	using namespace std;
//	typedef const Type * str_t;
//	typedef str_t (*func_t)(str_t, Type);
//	reverse_iterator<str_t> first1(&where[Len(where)]);
//	reverse_iterator<str_t> last1(&where[0]);
//	reverse_iterator<str_t> pos = find_if(first1, last1, tr1::bind((func_t)Find, what, _1));
////	reverse_iterator<str_t> pos = find_if(first1, last1, bind1st(ptr_fun<str_t, Type, str_t>(Find), what));
//	return (last1 == pos) ? nullptr : &(*pos);
//}
//
//template<typename Type>
//const Type * find_last_not_of(const Type * where, const Type * what) {
//	using namespace std;
//	typedef const Type * str_t;
//	typedef str_t (*func_t)(str_t, Type);
//	reverse_iterator<str_t> first1(&where[Len(where)]);
//	reverse_iterator<str_t> last1(&where[0]);
//	reverse_iterator<str_t> pos = find_if(first1, last1, tr1::bind(logical_not<bool>(), tr1::bind((func_t)Find, what, _1)));
////	reverse_iterator<str_t> pos = find_if(first1, last1, not1(bind1st(ptr_fun<str_t, Type, str_t>(Find), what)));
//	return (last1 == pos) ? nullptr : &(*pos);
//}

ustring as_str(const PBYTE buf, size_t size);

ustring as_str(auto_array<BYTE> buf);

auto_array<BYTE> as_hash(const ustring & str);

void as_hash(const ustring & str, PBYTE & buf, size_t & size);

astring Hash2Str(const PBYTE buf, size_t size);
astring Hash2StrNum(const PBYTE buf, size_t size);
bool Str2Hash(const astring & str, PVOID & hash, ULONG & size);

UINT CheckUnicode(const PVOID buf, size_t size);
UINT IsUTF8(const PVOID buf, size_t size);
bool GetCP(HANDLE hFile, UINT & cp, bool bUseHeuristics = false);

ustring as_str(const SYSTEMTIME & in, bool tolocal = true);
ustring as_str(const FILETIME & in);

ustring & to_lower(ustring & inout);
ustring & to_upper(ustring & inout);

///========================================================================================= BitMask
template<typename Type>
struct BitMask {
	static Type from_str(const ustring & in, size_t lim = 0) {
		// count bits from 1
		Type	Result = 0;
		intmax_t	bit = 0;
		ustring	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(--bit))
				WinBit::Set(Result, bit);
		}
		return Result;
	}

	static Type from_str_0(const ustring & in, size_t lim = 0) {
		// count bits from zero
		Type	Result = 0;
		ssize_t	bit = 0;
		ustring	tmp(in);
		lim = WinBit::Limit<Type>(lim);
		while (tmp.Cut(bit)) {
			if (!WinBit::BadBit<Type>(bit))
				WinBit::Set(Result, bit);
		}
		return Result;
	}

	static ustring	as_str(Type in, size_t lim = 0) {
		// count bits from 1
		ustring	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(as_str(bit + 1), L",");
			}
		}
		return Result;
	}

	static ustring	as_str_0(Type in, size_t lim = 0) {
		// count bits from zero
		ustring	Result;
		lim = WinBit::Limit<Type>(lim);
		for (size_t	bit = 0; bit < lim; ++bit) {
			if (WinBit::Check(in, bit)) {
				Result.Add(as_str(bit), L",");
			}
		}
		return Result;
	}

	static ustring	as_str_bin(Type in, size_t lim = 0) {
		ustring	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			Result += WinFlag::Check(in, (Type)flag) ? L'1' : L'0';
			flag >>= 1;
		}
		return Result;
	}

	static ustring	as_str_num(Type in, size_t lim = 0) {
		ustring	Result;
		uintmax_t	flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
		while (flag) {
			if (WinFlag::Check(in, (Type)flag)) {
				Result.Add(as_str(flag), L",");
			}
			flag >>= 1;
		}
		return Result;
	}
};

#endif
