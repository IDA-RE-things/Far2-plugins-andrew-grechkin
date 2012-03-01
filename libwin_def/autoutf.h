/**
	win_autoutf
	Manipulate string and wstring
	@classes	(AutoSTR, BitMask)
	@author		© 2009 Andrew Grechkin
	@link ()
**/
#ifndef WIN_AUTOUTF_HPP
#define WIN_AUTOUTF_HPP

#include <string>

///===================================================================================== definitions
inline std::string w2cp(PCWSTR in, UINT cp) {
	auto size = Convert(in, cp);
	auto_array<CHAR> buf(size);
	Convert(in, cp, buf.data(), size);
	return std::string(buf.data());
}

inline std::wstring cp2w(PCSTR in, UINT cp) {
	auto size = Convert(in, cp);
	auto_array<WCHAR> buf(size);
	Convert(in, cp, buf.data(), size);
	return std::wstring(buf.data());
}

///========================================================================================= AutoSTR
class AutoSTR: public std::wstring {
	typedef std::wstring base_type;

public:
	AutoSTR() {
	}
	AutoSTR(size_t len, WCHAR in):
		base_type(len, in) {
	}
	AutoSTR(const AutoSTR & in):
		base_type(base_type(in)) {
	}
	AutoSTR(const base_type & in):
		base_type(in) {
	}
	AutoSTR(PCWSTR in):
		base_type(in) {
	}
	AutoSTR(PCWSTR in, size_t n):
		base_type(in, n) {
	}
	AutoSTR(const std::string & in, UINT cp = DEFAULT_CP):
		base_type(cp2w(in.c_str(), cp)) {
	}
	AutoSTR(PCSTR in, UINT cp = DEFAULT_CP):
		base_type(cp2w(in, cp)) {
	}

	operator const std::string() const {
		return w2cp(c_str(), DEFAULT_CP);
	}

	operator std::string() {
		return w2cp(c_str(), DEFAULT_CP);
	}

//	using base_type::size;
//	using base_type::empty;
//	using base_type::assign;
//	using base_type::append;
//	using base_type::c_str;
//	using base_type::at;
//	using base_type::clear;
//	using base_type::find;
//	using base_type::rfind;
//	using base_type::operator =;
//	using base_type::operator +=;
//	using base_type::operator [];
};

//class AutoSTR {
//public:
//	typedef size_t size_type;
//	static const size_t npos = wstring::npos;
//
//	AutoSTR() {
//	}
//	AutoSTR(size_t len, WCHAR in): m_str(len, in) {
//	}
//	AutoSTR(const AutoSTR &in): m_str(in.m_str) {
//	}
//	AutoSTR(const wstring &in): m_str(in) {
//	}
//	AutoSTR(const string &in, UINT cp = DEFAULT_CP): m_str(cp2w(in.c_str(), cp)) {
//	}
//	AutoSTR(PCWSTR in): m_str(in) {
//	}
//	AutoSTR(PCWSTR in, size_t n): m_str(in, n) {
//	}
//	AutoSTR(PCSTR in, UINT cp = DEFAULT_CP): m_str(cp2w(in, cp)) {
//	}
//
//	operator const string() const {
//		return w2cp(m_str.c_str(), DEFAULT_CP);
//	}
//	operator string() {
//		return w2cp(m_str.c_str(), DEFAULT_CP);
//	}
//
//	operator const wstring&() const {
//		return m_str;
//	}
//	operator wstring() {
//		return m_str;
//	}
//
//	string cp(UINT cp_) const {
//		return w2cp(m_str.c_str(), cp_);
//	}
//	string utf8() const {
//		return cp(CP_UTF8);
//	}
//	string oem() const {
//		return cp(CP_OEMCP);
//	}
//	string ansi() const {
//		return cp(CP_ACP);
//	}
//	wstring & utf16() {
//		return m_str;
//	}
//	wstring utf16() const {
//		return m_str;
//	}
//
//	const AutoSTR & operator=(const AutoSTR &rhs) {
//		m_str = rhs.m_str;
//		return *this;
//	}
//	const AutoSTR & operator=(const wstring &rhs) {
//		m_str = rhs;
//		return *this;
//	}
//	const AutoSTR & operator=(PCWSTR rhs) {
//		m_str = rhs;
//		return *this;
//	}
//	bool operator ==(const AutoSTR &rhs) const {
//		return m_str == rhs.m_str;
//	}
//	bool operator ==(const wstring &rhs) const {
//		return m_str == rhs;
//	}
//	bool operator==(PCWSTR rhs) const {
//		return m_str == rhs;
//	}
//	bool operator!=(const AutoSTR &rhs) const {
//		return m_str != rhs.m_str;
//	}
//	bool operator!=(const wstring &rhs) const {
//		return m_str != rhs;
//	}
//	bool operator!=(PCWSTR rhs) const {
//		return m_str != rhs;
//	}
//	AutoSTR & operator+=(const wchar_t &rhs) {
//		m_str += rhs;
//		return *this;
//	}
//	AutoSTR & operator+=(const AutoSTR &rhs) {
//		m_str += rhs.m_str;
//		return *this;
//	}
//	AutoSTR & operator+=(const wstring &rhs) {
//		m_str += rhs;
//		return *this;
//	}
//	AutoSTR & operator+=(PCWSTR rhs) {
//		m_str += rhs;
//		return *this;
//	}
//	AutoSTR operator+(const AutoSTR &rhs) const {
//		AutoSTR tmp(*this);
//		return tmp += rhs;
//	}
//	AutoSTR operator+(const wstring &rhs) const {
//		AutoSTR tmp(*this);
//		return tmp += rhs;
//	}
//	AutoSTR operator+(PCWSTR rhs) const {
//		AutoSTR tmp(*this);
//		return tmp += rhs;
//	}
//	bool operator<(const AutoSTR &rhs) const {
//		return m_str < rhs.m_str;
//	}
//	bool operator<(const wstring &rhs) const {
//		return m_str < rhs;
//	}
//	bool operator<(PCWSTR rhs) const {
//		return m_str < rhs;
//	}
//
//	wstring::size_type	size() const {
//		return m_str.size();
//	}
//	wstring::size_type	len() const {
//		return m_str.length();
//	}
//	wstring::size_type	length() const {
//		return m_str.length();
//	}
//	wstring::size_type	max_size() const {
//		return m_str.max_size();
//	}
//	void				resize(wstring::size_type n) {
//		m_str.resize(n);
//	}
//	wstring::size_type	capacity() const {
//		return m_str.capacity();
//	}
//	void				reserve(wstring::size_type res_arg = 0) {
//		m_str.reserve(res_arg);
//	}
//	void				clear() {
//		m_str.clear();
//	}
//	bool				empty() const {
//		return m_str.empty();
//	}
//
//	const wchar_t&		operator[](int index) const {
//		return m_str[index];
//	}
//	wchar_t&			operator[](int index) {
//		return m_str[index];
//	}
//	const wchar_t&		at(wstring::size_type index) const {
//		return m_str.at(index);
//	}
//	wchar_t&			at(wstring::size_type index) {
//		return m_str.at(index);
//	}
//
//	AutoSTR&			assign(PCWSTR s) {
//		m_str.assign(s);
//		return *this;
//	}
//	AutoSTR&			assign(PCWSTR s, size_t n) {
//		m_str.assign(s, n);
//		return *this;
//	}
//	AutoSTR&			erase(wstring::size_type pos = 0, wstring::size_type n = wstring::npos) {
//		m_str.erase(pos, n);
//		return *this;
//	}
//	AutoSTR&			replace(wstring::size_type pos1, wstring::size_type n1, const AutoSTR& str) {
//		m_str.replace(pos1, n1, str);
//		return *this;
//	}
//
//	PCWSTR				c_str() const {
//		return m_str.c_str();
//	}
//	wstring::size_type	find(PCWSTR str, wstring::size_type pos = 0) const {
//		return m_str.find(str, pos);
//	}
//	wstring::size_type	find(const AutoSTR &str, wstring::size_type pos = 0) const {
//		return m_str.find(str, pos);
//	}
//	wstring::size_type	find(const WCHAR c, wstring::size_type pos = 0) const {
//		return m_str.find(c, pos);
//	}
//	wstring::size_type	rfind(PCWSTR str, wstring::size_type pos = wstring::npos) const {
//		return m_str.rfind(str, pos);
//	}
//	wstring::size_type	rfind(const AutoSTR &str, wstring::size_type pos = wstring::npos) const {
//		return m_str.rfind(str, pos);
//	}
//	wstring::size_type	rfind(const WCHAR c, wstring::size_type pos = wstring::npos) const {
//		return m_str.rfind(c, pos);
//	}
//	wstring::size_type	find_first_of(const AutoSTR &str, wstring::size_type pos = 0) const {
//		return m_str.find_first_of(str.c_str(), pos);
//	}
//	wstring::size_type	find_last_of(const AutoSTR &str, wstring::size_type pos = wstring::npos) const {
//		return m_str.find_last_of(str, pos);
//	}
//	wstring::size_type	find_first_not_of(const AutoSTR &str, wstring::size_type pos = 0) const {
//		return m_str.find_first_not_of(str, pos);
//	}
//	wstring::size_type	find_last_not_of(const AutoSTR &str, wstring::size_type pos = wstring::npos) const {
//		return m_str.find_last_not_of(str, pos);
//	}
//	AutoSTR				substr(wstring::size_type pos = 0, wstring::size_type n = wstring::npos) const {
//		return m_str.substr(pos, n);
//	}
//
//	bool		Find(wchar_t c, size_t pos = 0) const {
//		return this->find(c, pos) != wstring::npos;
//	}
//	bool		Find(const AutoSTR &sub) const {
//		return this->find(sub) != wstring::npos;
//	}
//	bool		Find(const AutoSTR &sub, wstring::size_type &pos) const {
//		pos = this->find(sub);
//		return pos != wstring::npos;
//	}
//
//private:
//	wstring m_str;
//};

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
typedef AutoSTR ustring;

#endif
