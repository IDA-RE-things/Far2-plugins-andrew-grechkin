/**
	win_autoutf
	Manipulate string and wstring
	@classes	(AutoSTR, BitMask)
	@author		© 2009 Andrew Grechkin
	@link ()
**/
#ifndef WIN_AUTOUTF_HPP
#define WIN_AUTOUTF_HPP

#include "std.h"
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
template<typename Type>
class AutoSTR {
	typedef AutoSTR this_type;
	typedef Type char_type;

public:
	typedef size_t size_type;

	static const size_type npos = ~(size_type)0;

	~AutoSTR() {
		delRef();
	}

	AutoSTR():
		m_data(alloc_cstr(1)) {
	}

	AutoSTR(size_type len, Type in):
		m_data(alloc_cstr(len + 1)) {
		init(in, len);
	}

	AutoSTR(const Type * in, size_t len = 0):
		m_data(alloc_cstr(((in && len == 0) ? (len = Len(in)) : len) + 1)) {
		init(in, len);
	}

	AutoSTR(const this_type & in):
		m_data(in.m_data) {
		addRef();
	}

	size_t capacity() const {
		return m_data->m_capa;
	}
	void clear() {
		split();
		m_data->m_size = 0;
		m_data->m_str[0] = 0;
	}
	bool empty() const {
		return m_data->m_size == 0;
	}
	void reserve(size_t capa) {
		if (capacity() < capa) {
			if (m_data->m_ref > 1) {
				this_type(capa, this).swap(*this);
			} else {
				WinMem::Realloc(m_data, sizeof(*m_data) + capa * sizeof(Type));
				m_data->m_capa = capa;
			}
		}
	}
	size_t size() const {
		return m_data->m_size;
	}

	const Type * c_str() const {
		return m_data->m_str;
	}
	Type * buffer() {
		split();
		return m_data->m_str;
	}
	void swap(this_type & in) {
		using std::swap;
		swap(m_data, in.m_data);
	}

	this_type & append(const this_type & str) {
		if (m_data != str.m_data) {
			append(str.c_str(), str.size());
		} else {
			this_type tmp(size() + str.size(), this);
			tmp.append(str.c_str(), str.size());
			swap(tmp);
		}
		return *this;
	}
	this_type & append(const this_type & str, size_t pos, size_t n = npos) {
		return append(&str[pos], n);
	}
	this_type & append(const Type * s, size_t n) {
		if (n) {
			reserve(size() + n);
			WinMem::Copy(m_data->m_str + size(), s, n * sizeof(Type));
			m_data->m_size += n;
			m_data->m_str[size()] = (Type)0;
		}
		return *this;
	}
	this_type & append(const Type *s) {
		return append(s, Len(s));
	}
	this_type & append(size_t n, Type c) {
		return append(this_type(n, c));
	}

	this_type & assign(const this_type & in) {
		if (this != &in) {
			delRef();
			m_data = in.m_data;
			addRef();
		}
		return *this;
	}
	this_type & assign(const this_type & str, size_t pos, size_t n = npos) {
		return assign(&str[pos], n);
	}
	this_type & assign(const Type * s, size_t n) {
		if (n) {
			reserve(n);
			WinMem::Copy(m_data->m_str, s, n * sizeof(Type));
			m_data->m_size = n;
			m_data->m_str[size()] = (Type)0;
		}
		return *this;
	}
	this_type & assign(const Type * s) {
		return assign(s, Len(s));
	}
	this_type & assign(size_t n, Type c) {
		return assign(this_type(n, c));
	}

	this_type & replace(size_t pos, size_t n1, const this_type & str) {
		if (pos <= size()) {
			this_type tmp(size() + str.size(), (const AutoSTR *)nullptr);
			tmp.append(m_data->m_str, pos);
			tmp.append(str);
			if ((pos + n1) < size())
				tmp.append(&m_data->m_str[pos + n1]);
			swap(tmp);
		}
		return *this;
	}
	this_type & erase(size_t pos = 0, size_t n = npos) {
		if (pos < size()) {
			size_t size2 = n == npos ? size() : std::min(size(), pos + n);
			this_type(c_str(), pos, c_str() + size2, size() - size2).swap(*this);
		}
		return *this;
	}
	this_type substr(size_t pos = 0, size_t n = npos) const {
		if (pos < size()) {
			return (n == npos) ?
				this_type(&m_data->m_str[pos])
			:
				this_type(&m_data->m_str[pos], n);
		}
		return *this;
	}

	const this_type & operator =(const this_type & in) {
		return assign(in);
	}
	this_type & operator +=(const this_type & in) {
		return append(in);
	}
	this_type & operator +=(const Type * in) {
		return append(in);
	}
	this_type & operator +=(Type in) {
		Type tmp[] = {in, 0};
		return append(tmp, 1);
	}

	this_type operator +(const this_type & in) const {
		this_type tmp(size() + in.size(), this);
		return tmp += in;
	}
	this_type operator +(const Type * in) const {
		this_type tmp(size() + Len(in), this);
		return tmp += in;
	}
	this_type operator +(Type in) const {
		this_type tmp(size() + 1);
		return tmp += in;
	}

	bool operator ==(const this_type & in) const {
		return Eq(c_str(), in.c_str());
	}
	bool operator ==(const Type * in) const {
		return Eq(c_str(), in);
	}
	bool operator!=(const this_type & in) const {
		return !operator ==(in);
	}
	bool operator !=(const Type * in) const {
		return !operator ==(in);
	}

	bool operator <(const this_type & in) const {
		return Cmp(c_str(), in.c_str()) < 0;
	}
	bool operator <(const Type * in) const {
		return Cmp(c_str(), in) < 0;
	}
	bool operator >(const this_type & in) const {
		return Cmp(c_str(), in.c_str()) > 0;
	}
	bool operator >(const Type * in) const {
		return Cmp(c_str(), in) > 0;
	}
	bool operator <=(const this_type & in) const {
		return operator ==(in) || operator <(in);
	}
	bool operator <=(const Type * in) const {
		return operator==(in) || operator <(in);
	}
	bool operator >=(const this_type & in) const {
		return operator ==(in) || operator >(in);
	}
	bool operator >=(const Type * in) const {
		return operator ==(in) || operator >(in);
	}

	Type & operator [] (int in) {
		split();
		return m_data->m_str[in];
	}
	const Type & operator [](int in) const {
		return m_data->m_str[in];
	}
	Type & at(size_t in) {
		split();
		return m_data->m_str[in];
	}
	const Type & at(size_t in) const {
		return m_data->m_str[in];
	}

	size_t find(Type c, size_t p = 0) const {
		Type what[] = {c, 0};
		return find(what, p);
	}
	size_t find(const this_type & in, size_t p = 0) const {
		return find(in.c_str(), p);
	}
	size_t find(const Type * s, size_t p = 0) const {
		const Type * pos = ::Find(c_str() + std::min(p, size()), s);
		if (pos) {
			return pos - c_str();
		}
		return npos;
	}
	size_t rfind(const this_type & in) const {
		const Type * pos = RFind(c_str(), in.c_str());
		if (pos)
			return pos - c_str();
		return npos;
	}

	size_t find_first_of(const this_type & str, size_t pos = 0) const {
		size_t	Result = Span(c_str() + pos, str.c_str());
		return (Result < size()) ? Result : npos;
	}
	size_t find_last_of(const this_type & str, size_t pos = npos) const {
		size_t	Result = Span(c_str() + pos, str.c_str());
		return (Result < size()) ? Result : npos;
	}
	size_t find_first_not_of(const this_type & str, size_t pos = 0) const {
		for (; pos < size(); ++pos)
			if (::Find(str.c_str(), at(pos)))
				return pos;
		return npos;
	}
	size_t find_last_not_of(const this_type & str, size_t pos = npos) const {
		size_t	__size = size();
		if (__size) {
			if (--__size > pos)
				__size = pos;
			do {
				if (!::Find(str.c_str(), at(__size)))
					return __size;
			} while (__size--);
		}
		return npos;
	}

//	bool			Find(Type c, size_t pos = 0) const {
//		return find(c, pos) != npos;
//	}
//	bool			Find(const this_type &sub) const {
//		return find(sub) != npos;
//	}
//	bool			Find(const this_type &sub, size_t &pos) const {
//		pos = find(sub);
//		return pos != npos;
//	}
//	this_type&		Add(const Type add)  {
//		size_t	pos = this->size() - 1;
//		if (!(empty() || (at(pos) == add)))
//			operator+=(add);
//		return *this;
//	}
//	this_type&		Add(const this_type &add)  {
//		size_t	pos = this->size() - add.size();
//		if (!(add.empty() || empty() || (rfind(add) == pos)))
//			this->operator+=(add);
//		return *this;
//	}
//	this_type&		Add(const this_type &add, const this_type &delim, bool chkEmpty = true) {
//		size_t	pos = size() - delim.size();
//		if (!(add.empty() || delim.empty() || (chkEmpty && empty()) || (rfind(delim) == pos) || (add.find(delim) == 0)))
//			operator+=(delim);
//		if (!add.empty())
//			operator+=(add);
//		return *this;
//	}
//	this_type&		Cut(const this_type &sub) {
//		size_t	pos;
//		if (Find(sub, pos)) {
//			erase(pos, sub.size());
//		}
//		return *this;
//	}
//	bool			Cut(intmax_t &num, int base = 10) {
//		size_t	pos1 = find_first_of(L"0123456789");
//		if (pos1 == npos)
//			return false;
//		size_t	pos2 = find_first_not_of(L"0123456789", pos1);
//		if (pos1 > 0 && at(pos1 - 1) == L'-')
//			--pos1;
//		AutoSTR	tmp(substr(pos1, pos2));
//		tmp.AsNum(num, base);
//		erase(0, pos2);
//		return true;
//	}

//	bool			AsNum(uintmax_t &num, int base = 10) const {
//		PWSTR	end_ptr;
//		num = ::wcstoll(c_str(), &end_ptr, base);
//		return end_ptr != c_str();
//	}
//	bool			AsNum(intmax_t &num, int base = 10) const {
//		PWSTR	end_ptr;
//		num = ::wcstoull(c_str(), &end_ptr, base);
//		return end_ptr != c_str();
//	}

private:
	struct Cont {
		size_t	m_ref;
		size_t	m_capa;
		size_t	m_size;
		Type	m_str[1];

		Cont():
			m_ref(1),
			m_capa(1),
			m_size(0) {
			m_str[0] = 0;
		}
	} * m_data;

	void delRef() {
		if (m_data && --m_data->m_ref == 0) {
			WinMem::Free(m_data);
		}
	}

	void addRef() {
		++m_data->m_ref;
	}

	void init(Type in, size_t len) {
		if (in != (Type)0) {
			std::fill(m_data->m_str, &m_data->m_str[len], in);
		}
		m_data->m_size = len;
		m_data->m_str[len] = (Type)0;
	}

	void init(const Type * in, size_t len) {
		WinMem::Copy(m_data->m_str, in, len * sizeof(Type));
		m_data->m_size = len;
		m_data->m_str[len] = (Type)0;
	}

	Cont * alloc_cstr(size_t capa) {
		Cont * ret = (Cont *)WinMem::Alloc(sizeof(Cont) + capa * sizeof(Type));
		ret->m_ref = 1;
		ret->m_capa = capa;
		return ret;
	}

	void split() {
		if (m_data->m_ref > 1)
			this_type(capacity(), this).swap(*this);
	}

	AutoSTR(size_t capa, const AutoSTR * str):
		m_data(alloc_cstr(capa)) {
		if (str)
			init(str->c_str(), str->size());
	}

	AutoSTR(const Type * str1, size_t size1, const Type * str2, size_t size2): m_data(alloc_cstr(size1 + size2 + 1)) {
		if (size1) {
			WinMem::Copy(m_data->m_str, str1, size1 * sizeof(Type));
			m_data->m_size = size1;
		}
		if (size2) {
			WinMem::Copy(m_data->m_str + size1, str2, size2 * sizeof(Type));
			m_data->m_size += size2;
		}
	}

	template<typename Type1>
	friend AutoSTR<Type1> operator +(const Type1 * lhs, const AutoSTR<Type1> & rhs);
};

template<typename Type>
inline AutoSTR<Type> operator +(const Type * lhs, const AutoSTR<Type> & rhs) {
	return AutoSTR<Type>(lhs, Len(lhs), rhs.c_str(), rhs.size());
}

typedef AutoSTR<CHAR> astring;
typedef AutoSTR<WCHAR> ustring;

inline astring	w2cp(PCWSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	auto_array<CHAR> buf(size);
	Convert(in, cp, buf, size);
	return astring(buf.data());
}

inline ustring	cp2w(PCSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	auto_array<WCHAR> buf(size);
	Convert(in, cp, buf, size);
	return ustring(buf.data());
}

#else

#include <string>

///===================================================================================== definitions
inline std::string w2cp(PCWSTR in, UINT cp) {
	size_t size = Convert(in, cp);
	auto_array<CHAR> buf(size);
	Convert(in, cp, buf.data(), size);
	return std::string(buf.data());
}

inline std::wstring cp2w(PCSTR in, UINT cp) {
	size_t size = Convert(in, cp);
	auto_array<WCHAR> buf(size);
	Convert(in, cp, buf.data(), size);
	return std::wstring(buf.data());
}

///========================================================================================= AutoSTR
//class AutoSTR: public std::wstring {
//	typedef std::wstring base_type;
//
//public:
//	AutoSTR() {
//	}
//	AutoSTR(size_t len, WCHAR in):
//		base_type(len, in) {
//	}
//	AutoSTR(const AutoSTR & in):
//		base_type(base_type(in)) {
//	}
//	AutoSTR(const base_type & in):
//		base_type(in) {
//	}
//	AutoSTR(PCWSTR in):
//		base_type(in) {
//	}
//	AutoSTR(PCWSTR in, size_t n):
//		base_type(in, n) {
//	}
//	AutoSTR(const std::string & in, UINT cp = DEFAULT_CP):
//		base_type(cp2w(in.c_str(), cp)) {
//	}
//	AutoSTR(PCSTR in, UINT cp = DEFAULT_CP):
//		base_type(cp2w(in, cp)) {
//	}
//
//	operator const std::string() const {
//		return w2cp(c_str(), DEFAULT_CP);
//	}
//
//	operator std::string() {
//		return w2cp(c_str(), DEFAULT_CP);
//	}
//
////	using base_type::size;
////	using base_type::empty;
////	using base_type::assign;
////	using base_type::append;
////	using base_type::c_str;
////	using base_type::at;
////	using base_type::clear;
////	using base_type::find;
////	using base_type::rfind;
////	using base_type::operator =;
////	using base_type::operator +=;
////	using base_type::operator [];
//};

class AutoSTR {
public:
	typedef size_t size_type;
	static const size_t npos = std::wstring::npos;

	AutoSTR() {
	}
	AutoSTR(size_t len, WCHAR in): m_str(len, in) {
	}
	AutoSTR(const AutoSTR &in): m_str(in.m_str) {
	}
	AutoSTR(const std::wstring &in): m_str(in) {
	}
	AutoSTR(const std::string &in, UINT cp = DEFAULT_CP): m_str(cp2w(in.c_str(), cp)) {
	}
	AutoSTR(PCWSTR in): m_str(in) {
	}
	AutoSTR(PCWSTR in, size_t n): m_str(in, n) {
	}
	AutoSTR(PCSTR in, UINT cp = DEFAULT_CP): m_str(cp2w(in, cp)) {
	}

	operator const std::string() const {
		return w2cp(m_str.c_str(), DEFAULT_CP);
	}
	operator std::string() {
		return w2cp(m_str.c_str(), DEFAULT_CP);
	}

	operator const std::wstring&() const {
		return m_str;
	}
	operator std::wstring() {
		return m_str;
	}

	std::string cp(UINT cp_) const {
		return w2cp(m_str.c_str(), cp_);
	}
	std::string utf8() const {
		return cp(CP_UTF8);
	}
	std::string oem() const {
		return cp(CP_OEMCP);
	}
	std::string ansi() const {
		return cp(CP_ACP);
	}
	std::wstring & utf16() {
		return m_str;
	}
	std::wstring utf16() const {
		return m_str;
	}

	const AutoSTR & operator=(const AutoSTR &rhs) {
		m_str = rhs.m_str;
		return *this;
	}
	const AutoSTR & operator=(const std::wstring &rhs) {
		m_str = rhs;
		return *this;
	}
	const AutoSTR & operator=(PCWSTR rhs) {
		m_str = rhs;
		return *this;
	}
	bool operator ==(const AutoSTR &rhs) const {
		return m_str == rhs.m_str;
	}
	bool operator ==(const std::wstring &rhs) const {
		return m_str == rhs;
	}
	bool operator==(PCWSTR rhs) const {
		return m_str == rhs;
	}
	bool operator!=(const AutoSTR &rhs) const {
		return m_str != rhs.m_str;
	}
	bool operator!=(const std::wstring &rhs) const {
		return m_str != rhs;
	}
	bool operator!=(PCWSTR rhs) const {
		return m_str != rhs;
	}
	AutoSTR & operator+=(const wchar_t &rhs) {
		m_str += rhs;
		return *this;
	}
	AutoSTR & operator+=(const AutoSTR &rhs) {
		m_str += rhs.m_str;
		return *this;
	}
	AutoSTR & operator+=(const std::wstring &rhs) {
		m_str += rhs;
		return *this;
	}
	AutoSTR & operator+=(PCWSTR rhs) {
		m_str += rhs;
		return *this;
	}
	AutoSTR operator+(const AutoSTR &rhs) const {
		AutoSTR tmp(*this);
		return tmp += rhs;
	}
	AutoSTR operator+(const std::wstring &rhs) const {
		AutoSTR tmp(*this);
		return tmp += rhs;
	}
	AutoSTR operator+(PCWSTR rhs) const {
		AutoSTR tmp(*this);
		return tmp += rhs;
	}
	bool operator<(const AutoSTR &rhs) const {
		return m_str < rhs.m_str;
	}
	bool operator<(const std::wstring &rhs) const {
		return m_str < rhs;
	}
	bool operator<(PCWSTR rhs) const {
		return m_str < rhs;
	}

	std::wstring::size_type	size() const {
		return m_str.size();
	}
	std::wstring::size_type	len() const {
		return m_str.length();
	}
	std::wstring::size_type	length() const {
		return m_str.length();
	}
	std::wstring::size_type	max_size() const {
		return m_str.max_size();
	}
	void				resize(std::wstring::size_type n) {
		m_str.resize(n);
	}
	std::wstring::size_type	capacity() const {
		return m_str.capacity();
	}
	void				reserve(std::wstring::size_type res_arg = 0) {
		m_str.reserve(res_arg);
	}
	void				clear() {
		m_str.clear();
	}
	bool				empty() const {
		return m_str.empty();
	}

	const wchar_t&		operator[](int index) const {
		return m_str[index];
	}
	wchar_t&			operator[](int index) {
		return m_str[index];
	}
	const wchar_t&		at(std::wstring::size_type index) const {
		return m_str.at(index);
	}
	wchar_t&			at(std::wstring::size_type index) {
		return m_str.at(index);
	}

	AutoSTR&			assign(PCWSTR s) {
		m_str.assign(s);
		return *this;
	}
	AutoSTR&			assign(PCWSTR s, size_t n) {
		m_str.assign(s, n);
		return *this;
	}
	AutoSTR&			erase(std::wstring::size_type pos = 0, std::wstring::size_type n = std::wstring::npos) {
		m_str.erase(pos, n);
		return *this;
	}
	AutoSTR&			replace(std::wstring::size_type pos1, std::wstring::size_type n1, const AutoSTR& str) {
		m_str.replace(pos1, n1, str);
		return *this;
	}

	PCWSTR				c_str() const {
		return m_str.c_str();
	}
	std::wstring::size_type	find(PCWSTR str, std::wstring::size_type pos = 0) const {
		return m_str.find(str, pos);
	}
	std::wstring::size_type	find(const AutoSTR &str, std::wstring::size_type pos = 0) const {
		return m_str.find(str, pos);
	}
	std::wstring::size_type	find(const WCHAR c, std::wstring::size_type pos = 0) const {
		return m_str.find(c, pos);
	}
	std::wstring::size_type	rfind(PCWSTR str, std::wstring::size_type pos = std::wstring::npos) const {
		return m_str.rfind(str, pos);
	}
	std::wstring::size_type	rfind(const AutoSTR &str, std::wstring::size_type pos = std::wstring::npos) const {
		return m_str.rfind(str, pos);
	}
	std::wstring::size_type	rfind(const WCHAR c, std::wstring::size_type pos = std::wstring::npos) const {
		return m_str.rfind(c, pos);
	}
	std::wstring::size_type	find_first_of(const AutoSTR &str, std::wstring::size_type pos = 0) const {
		return m_str.find_first_of(str.c_str(), pos);
	}
	std::wstring::size_type	find_last_of(const AutoSTR &str, std::wstring::size_type pos = std::wstring::npos) const {
		return m_str.find_last_of(str, pos);
	}
	std::wstring::size_type	find_first_not_of(const AutoSTR &str, std::wstring::size_type pos = 0) const {
		return m_str.find_first_not_of(str, pos);
	}
	std::wstring::size_type	find_last_not_of(const AutoSTR &str, std::wstring::size_type pos = std::wstring::npos) const {
		return m_str.find_last_not_of(str, pos);
	}
	AutoSTR				substr(std::wstring::size_type pos = 0, std::wstring::size_type n = std::wstring::npos) const {
		return m_str.substr(pos, n);
	}

	bool		Find(wchar_t c, size_t pos = 0) const {
		return this->find(c, pos) != std::wstring::npos;
	}
	bool		Find(const AutoSTR &sub) const {
		return this->find(sub) != std::wstring::npos;
	}
	bool		Find(const AutoSTR &sub, std::wstring::size_type &pos) const {
		pos = this->find(sub);
		return pos != std::wstring::npos;
	}

private:
	std::wstring m_str;
};

///=============================================================================== string <-> number
/*
template<typename Type>
Type			a2n(CONSTR &in) {
	istringstream is(in);
	Type t;
	is >> t;
	return (t);
}
template<typename Type>
Type			w2n(CONSTRW &in) {
	return a2n<Type>(w2a(in));
}
template<typename Type>
string			n2a(const Type &in) {
	ostringstream s;
	s << in;
	return (s.str());
}
template<typename Type>
wstring			n2w(const Type &in) {
	return a2w(n2a(in));
}
*/

///============================================================================================= Str
typedef std::string astring;
//typedef AutoSTR ustring;
typedef std::wstring ustring;

#endif

#endif
