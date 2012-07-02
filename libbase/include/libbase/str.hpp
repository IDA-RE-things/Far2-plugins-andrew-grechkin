#ifndef _LIBBASE_STR_HPP_
#define _LIBBASE_STR_HPP_

#include <libbase/pcstr.hpp>
#include <libbase/memory.hpp>
#include <libbase/autoutf.hpp>

namespace Base {

	inline size_t get_str_len(const astring &in) {
		return in.size();
	}

	inline size_t get_str_len(const ustring &in) {
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

	astring & Trim_l(astring &str, const astring &chrs = " \t\r\n");

	astring & Trim_r(astring &str, const astring &chrs = " \t\r\n");

	astring & Trim(astring &str, const astring &chrs = " \t\r\n");

	astring TrimOut(const astring &str, const astring &chrs = " \t\r\n");

	ustring & Trim_l(ustring &str, const ustring &chrs = L" \t\r\n");

	ustring & Trim_r(ustring &str, const ustring &chrs = L" \t\r\n");

	ustring & Trim(ustring &str, const ustring &chrs = L" \t\r\n");

	ustring TrimOut(const ustring &str, const ustring &chrs = L" \t\r\n");

	ustring GetWord(const ustring &str, WCHAR d = PATH_SEPARATOR_C);

	astring & AddWord(astring &inout, const astring &add, const astring &delim = "");

	ustring & AddWord(ustring &inout, const ustring &add, const ustring &delim = L"");

	astring & AddWordEx(astring &inout, const astring &add, const astring &delim = "");

	ustring & AddWordEx(ustring &inout, const ustring &add, const ustring &delim = L"");

	astring CutWord(astring &inout, const astring &delim = "\t ", bool delDelim = true);

	ustring CutWord(ustring & inout, const ustring &delim = L"\t ", bool delDelim = true);

	astring CutWordEx(astring & inout, const astring & delim, bool delDelim = true);

	ustring CutWordEx(ustring & inout, const ustring & delim, bool delDelim = true);

	ustring & ReplaceAll(ustring & str, const ustring & from, const ustring & to);

	ustring ReplaceAllOut(const ustring& str, const ustring &from, const ustring &to);

	ustring & Add(ustring & str, const wchar_t add);

	ustring & Add(ustring & str, const ustring & add);

	ustring & Add(ustring & str, const ustring & add, const ustring & delim, bool chkEmpty = true);

	ustring & Cut(ustring & str, const ustring & sub);

	bool Cut(ustring & str, intmax_t & num, int base = 10);

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

	ustring copy_after_last(const ustring & in, const ustring & delim);

	ustring & to_lower(ustring & inout);
	ustring & to_upper(ustring & inout);

}

#endif
