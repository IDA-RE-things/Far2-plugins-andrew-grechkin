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
#include <ostream>

///===================================================================================== definitions
using std::string;
using std::wstring;
using std::ostream;

typedef const string	CONSTR;
typedef const wstring	CONSTRW;

inline string			w2cp(PCWSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	CHAR	buf[size];
	Convert(in, cp, buf, size);
	return	buf;
}
inline wstring			cp2w(PCSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	WCHAR	buf[size];
	Convert(in, cp, buf, size);
	return	buf;
}


///========================================================================================= AutoUTF
class		AutoUTF {
	wstring		m_str;
public:
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
	AutoUTF&			Add(const wchar_t add);
	AutoUTF&			Add(const AutoUTF &add);
	AutoUTF&			Add(const AutoUTF &add, const AutoUTF &delim, bool chkEmpty = true);
	AutoUTF&			Cut(const AutoUTF &sub);
	bool				Cut(ssize_t &num, int base = 10);
	AutoUTF				CutWord(const AutoUTF &delim = L"\t ", bool delDelim = true);

	AutoUTF&			ReplaceAll(const AutoUTF &from, const AutoUTF &to);
	AutoUTF				ReplaceAllOut(const AutoUTF &from, const AutoUTF &to) const {
		AutoUTF	tmp(*this);
		return	tmp.ReplaceAll(from, to);
	}

	AutoUTF&			Trim_l(const AutoUTF &chrs = L" \t\r\n");
	AutoUTF&			Trim_r(const AutoUTF &chrs = L" \t\r\n");
	AutoUTF&			Trim(const AutoUTF &chrs = L" \t\r\n") {
		Trim_r(chrs);
		Trim_l(chrs);
		return	*this;
	}
	AutoUTF				TrimOut(const AutoUTF &chrs = L" \t\r\n") const {
		AutoUTF	tmp(*this);
		return	tmp.Trim(chrs);
	}

	bool				AsNum(size_t &num, int base = 10) const;
	bool				AsNum(ssize_t &num, int base = 10) const;

	static	AutoUTF		TrimOut(const AutoUTF &in, const AutoUTF &chrs = L" \t\r\n") {
		AutoUTF	out(in);
		return	out.Trim(chrs);
	}

//	path operations
	AutoUTF&			SlashAdd(const wchar_t c = PATH_SEPARATOR_C) {
		Add(c);
		return	*this;
	}
	AutoUTF&			SlashAddNec() {
		if (Find(L'/'))
			return	SlashAdd(L'/');
		return	SlashAdd(L'\\');
	}
	AutoUTF&			SlashDel() {
		if (!this->empty()) {
			wstring::size_type	pos = size() - 1;
			if (at(pos) == L'\\' || at(pos) == L'/')
				erase(pos);
		}
		return	*this;
	}
	AutoUTF&			PathWin() {
		return	ReplaceAll(L"/", L"\\");
	}
	AutoUTF&			PathUnix() {
		return	ReplaceAll(L"\\", L"/");
	}
};

inline size_t			Len(const AutoUTF &in) {
	return	in.size();
}

///=============================================================================== string <-> number
unsigned long long		s2ull(CONSTR &in, int base = 10);
long long				s2ll(CONSTR &in, int base = 10);
double					s2d(CONSTR &in);
inline size_t			s2ul(CONSTR &in, int base = 10) {
	return	s2ull(in, base);
}
inline ssize_t			s2l(CONSTR &in, int base = 10) {
	return	s2ll(in, base);
}

unsigned long long		s2ull(CONSTRW &in, int base = 10);
long long				s2ll(CONSTRW &in, int base = 10);
double					s2d(CONSTRW &in);
inline size_t			s2ul(CONSTRW &in, int base = 10) {
	return	s2ull(in, base);
}
inline ssize_t			s2l(CONSTRW &in, int base = 10) {
	return	s2ll(in, base);
}

//string					n2a(unsigned long long in, int base = 10);
string					n2a(intmax_t in, int base = 10);
string					d2a(double in);

//AutoUTF					n2w(unsigned long long in, int base = 10);
AutoUTF					n2w(intmax_t in, int base = 10);
AutoUTF					d2w(double in);

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

///================================================================================= String Utilites
namespace	StrUtil {

// CutWord
string				CutWord(string &inout, CONSTR &delim = "\t ", bool dd = true);
wstring				CutWord(wstring &inout, CONSTRW &delim = L"\t ", bool dd = true);

string				CutWordEx(string &inout, CONSTR &delim = "\t ", bool dd = true);
wstring				CutWordEx(wstring &inout, CONSTRW &delim = L"\t ", bool dd = true);

// AddWord
string&				AddWord(string &inout, CONSTR &add, CONSTR &delim = "");
wstring&			AddWord(wstring &inout, CONSTRW &add, CONSTRW &delim = L"");
string&				AddWordEx(string &inout, CONSTR &add, CONSTR &delim = "");
wstring&			AddWordEx(wstring &inout, CONSTRW &add, CONSTRW &delim = L"");
}

///=============================================================================== wstring extractor
#ifdef _GLIBCXX_OSTREAM
ostream&				operator<<(ostream &s, const AutoUTF &rhs);
#endif

///============================================================================================= Str
inline string			w2cp(const AutoUTF &in, UINT cp) {
	return	in.cp(cp);
}
inline string			w2u(const AutoUTF &in) {
	return	in.cp(CP_UTF8);
}

AutoUTF					ErrAsStr(DWORD err = ::GetLastError(), PCWSTR lib = NULL);
inline AutoUTF			ErrWmiAsStr(DWORD err) {
	return	ErrAsStr(err, L"wmiutils.dll");
}

AutoUTF					AsStr(const SYSTEMTIME &in, bool tolocal = true);
AutoUTF					AsStr(const FILETIME &in);

AutoUTF					AsStr(size_t num, int base = 10);
AutoUTF					AsStr(ssize_t &num, int base = 10);

inline bool				AsNum(const AutoUTF &str, size_t &num, int base = 10) {
	return	str.AsNum(num, base);
}
inline bool				AsNum(const AutoUTF &str, ssize_t &num, int base = 10) {
	return	str.AsNum(num, base);
}

AutoUTF					CopyAfterLast(const AutoUTF &in, const AutoUTF &delim);

AutoUTF&				Cut(AutoUTF &inout, const AutoUTF &in);
bool					Cut(AutoUTF &inout, ssize_t &num, int base = 10);
AutoUTF&				CutAfter(AutoUTF &inout, const AutoUTF &delim);
AutoUTF&				CutBefore(AutoUTF &inout, const AutoUTF &delim);

inline AutoUTF&			Trim_l(AutoUTF &inout, const AutoUTF &chrs = L" \t\r\n") {
	return	inout.Trim_l(chrs);
}
inline AutoUTF&			Trim_r(AutoUTF &inout, const AutoUTF &chrs = L" \t\r\n") {
	return	inout.Trim_r(chrs);
}
inline AutoUTF&			Trim(AutoUTF &inout, const AutoUTF &chrs = L" \t\r\n") {
	return	inout.Trim(chrs);
}
inline AutoUTF			TrimOut(const AutoUTF &in, const AutoUTF &chrs = L" \t\r\n") {
	return	AutoUTF::TrimOut(in, chrs);
}

AutoUTF&				ToLower(AutoUTF &inout);
AutoUTF					ToLowerOut(const AutoUTF &in);
AutoUTF&				ToUpper(AutoUTF &inout);
AutoUTF					ToUpperOut(const AutoUTF &in);

///========================================================================================= BitMask
template<typename Type>
class		BitMask : private WinBit<Type> {
public:
	using	WinBit<Type>::BIT_LIMIT;
	using	WinBit<Type>::BadBit;
	using	WinBit<Type>::Limit;
	using	WinBit<Type>::Check;
	using	WinBit<Type>::Set;
	using	WinBit<Type>::UnSet;

	static	Type		FromStr(const AutoUTF &in, size_t lim = 0) {
		// count bits from 1
		Type	Result = 0;
		lim = Limit(lim);
		ssize_t	bit = 0;
		AutoUTF	tmp(in);
		while (tmp.Cut(bit)) {
			if ((bit > 0) && (bit <= (ssize_t)lim))
				Set(Result, --bit);
		}
		return	Result;
	}
	static	Type		FromStr0(const AutoUTF &in, size_t lim = 0) {
		// count bits from zero
		Type	Result = 0;
		lim = Limit(lim);
		ssize_t	bit = 0;
		AutoUTF	tmp(in);
		while (tmp.Cut(bit)) {
			if ((bit >= 0) && (bit < lim))
				Set(Result, bit);
		}
		return	Result;
	}

	static	AutoUTF		AsStr(Type in, size_t lim = 0) {
		// count bits from 1
		AutoUTF	Result;
		lim = Limit(lim);
		for (size_t bit = 0; bit < lim; ++bit) {
			if (Check(in, bit)) {
				Result.Add(n2w(bit + 1), L",");
			}
		}
		return	Result;
	}
	static	AutoUTF		AsStr0(Type in, size_t lim = 0) {
		// count bits from zero
		AutoUTF	Result;
		lim = Limit(lim);
		for (size_t	bit = 0; bit < lim; ++bit) {
			if (Check(in, bit)) {
				Result.Add(n2w(bit), L",");
			}
		}
		return	Result;
	}
	static	AutoUTF		AsStrBin(Type in, size_t lim = 0) {
		AutoUTF	Result;
		uintmax_t	flag = (uintmax_t)1 << (Limit(lim) - 1);
		while (flag) {
			Result += WinFlag<Type>::Check(in, flag) ? L'1' : L'0';
			flag >>= 1;
		}
		return	Result;
	}
	static	AutoUTF		AsStrNum(Type in, size_t lim = 0) {
		AutoUTF	Result;
		uintmax_t	flag = (uintmax_t)1 << (Limit(lim) - 1);
		while (flag) {
			if (WinFlag<Type>::Check(in, flag)) {
				Result.Add(n2w(flag), L",");
			}
			flag >>= 1;
		}
		return	Result;
	}
};

typedef	std::string		CStrA;

#endif // WIN_AUTOUTF_HPP
