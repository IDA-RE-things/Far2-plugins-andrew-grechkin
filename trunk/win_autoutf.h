/**
	win_autoutf
	Manipulate string and wstring
	@classes	(AutoUTF, BitMask)
	@author		© 2009 Andrew Grechkin
	@link ()
**/
#ifndef WIN_AUTOUTF_HPP
#define WIN_AUTOUTF_HPP

#include <string>

///===================================================================================== definitions
using std::string;
using std::wstring;
using std::ostream;

typedef const string	CONSTR;
typedef const wstring	CONSTRW;

inline string	w2cp(PCWSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	CHAR	buf[size];
	Convert(in, cp, buf, size);
	return	string(buf);
}

inline wstring	cp2w(PCSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	WCHAR	buf[size];
	Convert(in, cp, buf, size);
	return	wstring(buf);
}


///========================================================================================= AutoUTF
class		AutoUTF {
	wstring		m_str;
public:
	typedef size_t size_type;
	static const	size_t	npos = wstring::npos;
	AutoUTF() {
	}
	AutoUTF(size_t len, WCHAR in): m_str(len, in) {
	}
	AutoUTF(const AutoUTF &in): m_str(in.m_str) {
	}
	AutoUTF(const wstring &in): m_str(in) {
	}
	AutoUTF(const string &in, UINT cp = DEFAULT_CHAR_CP): m_str(cp2w(in.c_str(), cp)) {
	}
	AutoUTF(PCWSTR in): m_str(in) {
	}
	AutoUTF(PCWSTR in, size_t n): m_str(in, n) {
	}
	AutoUTF(PCSTR in, UINT cp = DEFAULT_CHAR_CP): m_str(cp2w(in, cp)) {
	}

	operator			const string() const {
		return	w2cp(m_str.c_str(), DEFAULT_CHAR_CP);
	}
	operator			string() {
		return	w2cp(m_str.c_str(), DEFAULT_CHAR_CP);
	}

	operator			const wstring&() const {
		return	m_str;
	}
	operator			wstring() {
		return	m_str;
	}

	string				cp(UINT cp_) const {
		return	w2cp(m_str.c_str(), cp_);
	}
	string				utf8() const {
		return	cp(CP_UTF8);
	}
	string				oem() const {
		return	cp(CP_OEMCP);
	}
	string				ansi() const {
		return	cp(CP_ACP);
	}
	wstring&			utf16() {
		return	m_str;
	}
	wstring				utf16() const {
		return	m_str;
	}

	const AutoUTF&		operator=(const AutoUTF &rhs) {
		m_str = rhs.m_str;
		return	*this;
	}
	const AutoUTF&		operator=(const wstring &rhs) {
		m_str = rhs;
		return	*this;
	}
	const AutoUTF&		operator=(PCWSTR rhs) {
		m_str = rhs;
		return	*this;
	}
	bool				operator==(const AutoUTF &rhs) const {
		return	m_str == rhs.m_str;
	}
	bool				operator==(const wstring &rhs) const {
		return	m_str == rhs;
	}
	bool				operator==(PCWSTR rhs) const {
		return	m_str == rhs;
	}
	bool				operator!=(const AutoUTF &rhs) const {
		return	m_str != rhs.m_str;
	}
	bool				operator!=(const wstring &rhs) const {
		return	m_str != rhs;
	}
	bool				operator!=(PCWSTR rhs) const {
		return	m_str != rhs;
	}
	AutoUTF&			operator+=(const wchar_t &rhs) {
		m_str += rhs;
		return	*this;
	}
	AutoUTF&			operator+=(const AutoUTF &rhs) {
		m_str += rhs.m_str;
		return	*this;
	}
	AutoUTF&			operator+=(const wstring &rhs) {
		m_str += rhs;
		return	*this;
	}
	AutoUTF&			operator+=(PCWSTR rhs) {
		m_str += rhs;
		return	*this;
	}
	AutoUTF				operator+(const AutoUTF &rhs) const {
		AutoUTF	tmp(*this);
		return	tmp += rhs;
	}
	AutoUTF				operator+(const wstring &rhs) const {
		AutoUTF	tmp(*this);
		return	tmp += rhs;
	}
	AutoUTF				operator+(PCWSTR rhs) const {
		AutoUTF	tmp(*this);
		return	tmp += rhs;
	}
	bool				operator<(const AutoUTF &rhs) const {
		return	m_str < rhs.m_str;
	}
	bool				operator<(const wstring &rhs) const {
		return	m_str < rhs;
	}
	bool				operator<(PCWSTR rhs) const {
		return	m_str < rhs;
	}

	wstring::size_type	size() const {
		return	m_str.size();
	}
	wstring::size_type	len() const {
		return	m_str.length();
	}
	wstring::size_type	length() const {
		return	m_str.length();
	}
	wstring::size_type	max_size() const {
		return	m_str.max_size();
	}
	void				resize(wstring::size_type n) {
		m_str.resize(n);
	}
	wstring::size_type	capacity() const {
		return	m_str.capacity();
	}
	void				reserve(wstring::size_type res_arg = 0) {
		m_str.reserve(res_arg);
	}
	void				clear() {
		m_str.clear();
	}
	bool				empty() const {
		return	m_str.empty();
	}

	const wchar_t&		operator[](int index) const {
		return	m_str[index];
	}
	wchar_t&			operator[](int index) {
		return	m_str[index];
	}
	const wchar_t&		at(wstring::size_type index) const {
		return	m_str.at(index);
	}
	wchar_t&			at(wstring::size_type index) {
		return	m_str.at(index);
	}

	AutoUTF&			erase(wstring::size_type pos = 0, wstring::size_type n = wstring::npos) {
		m_str.erase(pos, n);
		return	*this;
	}
	AutoUTF&			replace(wstring::size_type pos1, wstring::size_type n1, const AutoUTF& str) {
		m_str.replace(pos1, n1, str);
		return	*this;
	}

	PCWSTR				c_str() const {
		return	m_str.c_str();
	}
	wstring::size_type	find(const AutoUTF &str, wstring::size_type pos = 0) const {
		return	m_str.find(str, pos);
	}
	wstring::size_type	find(char c, wstring::size_type pos = 0) const {
		return	m_str.find(c, pos);
	}
	wstring::size_type	rfind(const AutoUTF &str, wstring::size_type pos = wstring::npos) const {
		return	m_str.rfind(str, pos);
	}
	wstring::size_type	find_first_of(const AutoUTF &str, wstring::size_type pos = 0) const {
		return	m_str.find_first_of(str.c_str(), pos);
	}
	wstring::size_type	find_last_of(const AutoUTF &str, wstring::size_type pos = wstring::npos) const {
		return	m_str.find_last_of(str, pos);
	}
	wstring::size_type	find_first_not_of(const AutoUTF &str, wstring::size_type pos = 0) const {
		return	m_str.find_first_not_of(str, pos);
	}
	wstring::size_type	find_last_not_of(const AutoUTF &str, wstring::size_type pos = wstring::npos) const {
		return	m_str.find_last_not_of(str, pos);
	}
	AutoUTF				substr(wstring::size_type pos = 0, wstring::size_type n = wstring::npos) const {
		return	m_str.substr(pos, n);
	}

	/*
	class		AutoUTF: public wstring {
	public:
		AutoUTF(): wstring() {
		}
		AutoUTF(const AutoUTF &in): wstring(in) {
		}
		AutoUTF(PCWSTR in): wstring(in) {
		}
		AutoUTF(const wstring &in): wstring(in) {
		}
		AutoUTF(const string &in, UINT cp = DEFAULT_CHAR_CP): wstring(cp2w(in, cp)) {
		}
		AutoUTF(PCSTR in, UINT cp = DEFAULT_CHAR_CP): wstring(cp2w(in, cp)) {
		}

		operator			const string() const {
			return	w2cp(*this, DEFAULT_CHAR_CP);
		}
		operator			string() {
			return	w2cp(*this, DEFAULT_CHAR_CP);
		}
		operator			const wstring&() const {
			return	*this;
		}
		operator			wstring() {
			return	*this;
		}

		string				cp(UINT cp_) const {
			return	w2cp(*this, cp_);
		}
		string				utf8() const {
			return	w2cp(*this, CP_UTF8);
		}
		string				oem() const {
			return	w2cp(*this, CP_OEMCP);
		}
		string				ansi() const {
			return	w2cp(*this, CP_ACP);
		}
		wstring&			utf16() {
			return	*this;
		}
		wstring				utf16() const {
			return	*this;
		}

		const AutoUTF&		operator=(const AutoUTF &rhs) {
			(wstring)*this = rhs;
			return	*this;
		}
		const AutoUTF&		operator=(const wstring &rhs) {
			(wstring)*this = rhs;
			return	*this;
		}
		const AutoUTF&		operator=(PCWSTR rhs) {
			(wstring)*this = rhs;
			return	*this;
		}
		bool				operator==(const AutoUTF &rhs) const {
			return	(wstring)*this == rhs;
		}
		bool				operator==(const wstring &rhs) const {
			return	(wstring)*this == rhs;
		}
		bool				operator==(PCWSTR rhs) const {
			return	(wstring)*this == rhs;
		}
		bool				operator!=(const AutoUTF &rhs) const {
			return	(wstring)*this != rhs;
		}
		bool				operator!=(const wstring &rhs) const {
			return	(wstring)*this != rhs;
		}
		bool				operator!=(PCWSTR rhs) const {
			return	(wstring)*this != rhs;
		}
		AutoUTF&			operator+=(const AutoUTF &rhs) {
			(wstring)*this += rhs;
			return	*this;
		}
		AutoUTF&			operator+=(const wstring &rhs) {
			(wstring)*this += rhs;
			return	*this;
		}
		AutoUTF&			operator+=(PCWSTR rhs) {
			(wstring)*this += rhs;
			return	*this;
		}
		AutoUTF				operator+(const AutoUTF &rhs) const {
			AutoUTF	tmp(*this);
			return	tmp += rhs;
		}
		AutoUTF				operator+(const wstring &rhs) const {
			AutoUTF	tmp(*this);
			return	tmp += rhs;
		}
		AutoUTF				operator+(PCWSTR rhs) const {
			AutoUTF	tmp(*this);
			return	tmp += rhs;
		}
		bool				operator<(const AutoUTF &rhs) {
			return	(wstring)*this < rhs;
		}
		bool				operator<(const wstring &rhs) {
			return	(wstring)*this < rhs;
		}
		bool				operator<(PCWSTR rhs) {
			return	(wstring)*this < rhs;
		}
	*/

	bool				Find(wchar_t c, size_t pos = 0) const {
		return	this->find(c, pos) != wstring::npos;
	}
	bool				Find(const AutoUTF &sub) const {
		return	this->find(sub) != wstring::npos;
	}
	bool				Find(const AutoUTF &sub, wstring::size_type &pos) const {
		pos = this->find(sub);
		return	pos != wstring::npos;
	}
	AutoUTF&			Add(const wchar_t add) {
		wstring::size_type	pos = this->size() - 1;
		if (!(this->empty() || (m_str.at(pos) == add)))
			m_str += add;
		return	*this;
	}
	AutoUTF&			Add(const AutoUTF &add) {
		size_t	pos = this->size() - add.size();
		if (!(add.empty() || this->empty() || (this->rfind(add) == pos)))
			this->operator+=(add);
		return	*this;
	}
	AutoUTF&			Add(const AutoUTF &add, const AutoUTF &delim, bool chkEmpty = true) {
		size_t	pos = this->size() - delim.size();
		if (!(add.empty() || delim.empty() || (chkEmpty && this->empty()) || (this->rfind(delim) == pos) || (add.find(delim) == 0)))
			this->operator+=(delim);
		if (!add.empty())
			this->operator+=(add);
		return	*this;
	}
	AutoUTF&			Cut(const AutoUTF &sub) {
		wstring::size_type	pos;
		if (Find(sub, pos)) {
			this->erase(pos, sub.size());
		}
		return	*this;
	}
	bool				Cut(ssize_t &num, int base = 10) {
		size_t	pos1 = m_str.find_first_of(L"0123456789");
		if (pos1 == wstring::npos)
			return	false;
		size_t	pos2 = m_str.find_first_not_of(L"0123456789", pos1);
		if (pos1 > 0 && m_str[pos1-1] == L'-')
			--pos1;
		AutoUTF	tmp(m_str.substr(pos1, pos2));
		num = AsInt64(tmp.c_str(), base);
		m_str.erase(0, pos2);
		return	true;
	}

	AutoUTF&	SlashAdd(WCHAR c = PATH_SEPARATOR_C) {
		Add(c);
		return	*this;
	}

	AutoUTF&	SlashAddNec() {
		if (Find(L'/'))
			return	SlashAdd(L'/');
		return	SlashAdd(L'\\');
	}

	AutoUTF&	SlashDel() {
		if (!this->empty()) {
			wstring::size_type	pos = size() - 1;
			if (at(pos) == L'\\' || at(pos) == L'/')
				erase(pos);
		}
		return	*this;
	}
};

///=============================================================================== string <-> number
/*
template<typename Type>
Type			a2n(CONSTR &in) {
	istringstream is(in);
	Type t;
	is >> t;
	return	(t);
}
template<typename Type>
Type			w2n(CONSTRW &in) {
	return	a2n<Type>(w2a(in));
}
template<typename Type>
string			n2a(const Type &in) {
	ostringstream s;
	s << in;
	return	(s.str());
}
template<typename Type>
wstring			n2w(const Type &in) {
	return	a2w(n2a(in));
}
*/

///=============================================================================== wstring extractor
#include <ostream>
#ifdef _GLIBCXX_OSTREAM
ostream&	operator<<(ostream &s, const AutoUTF &rhs);
#endif

///============================================================================================= Str
inline string	w2cp(const AutoUTF &in, UINT cp) {
	return	in.cp(cp);
}
inline string	w2u(const AutoUTF &in) {
	return	in.cp(CP_UTF8);
}

typedef std::string CStrA;

#endif // WIN_AUTOUTF_HPP
