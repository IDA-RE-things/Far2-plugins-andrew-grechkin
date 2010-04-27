/**
	win_str
	Manipulate string and wstring
	@classes	(AutoUTF, BitMask)
	@author		© 2009 Andrew Grechkin
	@link ()
**/

#ifndef NoStlString
#include "win_def.h"

///=============================================================================== wstring extractor
#ifdef	_GLIBCXX_OSTREAM
ostream&				operator<<(ostream &s, const AutoUTF &rhs) {
	return	s << rhs.oem();
}
#endif

///=============================================================================== string <-> number
unsigned long long	s2ull(CONSTR &in, int base) {
	PSTR	end_ptr;
	return	::strtoul(in.c_str(), &end_ptr, base);
}
long long			s2ll(CONSTR &in, int base) {
	PSTR	end_ptr;
	return	::strtol(in.c_str(), &end_ptr, base);
}
double				s2d(CONSTR &in) {
	PSTR	end_ptr;
	return	::strtod(in.c_str(), &end_ptr);
}

unsigned long long	s2ull(CONSTRW &in, int base) {
	PWSTR	end_ptr;
	return	::wcstoul(in.c_str(), &end_ptr, base);
}
long long			s2ll(CONSTRW &in, int base) {
	PWSTR	end_ptr;
	return	::wcstol(in.c_str(), &end_ptr, base);
}
double				s2d(CONSTRW &in) {
	PWSTR	end_ptr;
	return	::wcstod(in.c_str(), &end_ptr);
}

//string				n2a(unsigned long long in, int base) {
//	CHAR	buf[MAX_PATH];
//	::ulltoa(in, buf, base);
//	return	buf;
//}
string				n2a(long long in, int base) {
	CHAR	buf[MAX_PATH];
	::lltoa(in, buf, base);
	return	buf;
}
string				d2a(double in) {
	CHAR	buf[MAX_PATH];
	::_gcvt(in, 12, buf);
	return	buf;
}

//AutoUTF				n2w(unsigned long long in, int base) {
//	WCHAR	buf[MAX_PATH];
//	::ulltow(in, buf, base);
//	return	buf;
//}
AutoUTF				n2w(long long in, int base) {
	WCHAR	buf[MAX_PATH];
	::lltow(in, buf, base);
	return	buf;
}
AutoUTF				d2w(double in, int base) {
	return	n2a(in);
}

///========================================================================================= AutoUTF
AutoUTF&			AutoUTF::Add(const wchar_t add) {
	wstring::size_type	pos = this->size() - 1;
	if (!(this->empty() || (m_str.at(pos) == add)))
		m_str += add;
	return	*this;
}
AutoUTF&			AutoUTF::Add(const AutoUTF &add) {
	size_t	pos = this->size() - add.size();
	if (!(add.empty() || this->empty() || (this->rfind(add) == pos)))
		this->operator+=(add);
	return	*this;
}
AutoUTF&			AutoUTF::Add(const AutoUTF &add, const AutoUTF &delim, bool chkEmpty) {
	size_t	pos = this->size() - delim.size();
	if (!(add.empty() || delim.empty() || (chkEmpty && this->empty()) || (this->rfind(delim) == pos) || (add.find(delim) == 0)))
		this->operator+=(delim);
	if (!add.empty())
		this->operator+=(add);
	return	*this;
}
AutoUTF&			AutoUTF::Cut(const AutoUTF &sub) {
	wstring::size_type	pos;
	if (Find(sub, pos)) {
		this->erase(pos, sub.size());
	}
	return	*this;
}
bool				AutoUTF::Cut(ssize_t &num, int base) {
	size_t	pos1 = m_str.find_first_of(L"0123456789");
	if (pos1 == wstring::npos)
		return	false;
	size_t	pos2 = m_str.find_first_not_of(L"0123456789", pos1);
	if (pos1 > 0 && m_str[pos1-1] == L'-')
		--pos1;
	AutoUTF	tmp(m_str.substr(pos1, pos2));
	tmp.AsNum(num, base);
	m_str.erase(0, pos2);
	return	true;
}
AutoUTF				AutoUTF::CutWord(const AutoUTF &delim, bool delDelim) {
	size_t	pos = this->find(delim);
	AutoUTF		Result(this->substr(0, pos));
	if (delDelim && pos != wstring::npos)
		pos += delim.size();
	this->erase(0, pos);
	return	Result;
}

AutoUTF&			AutoUTF::ReplaceAll(const AutoUTF &from, const AutoUTF &to) {
	wstring::size_type pos;
	while (Find(from, pos)) {
		this->replace(pos, from.size(), to);
	}
	return	*this;
}

AutoUTF&			AutoUTF::Trim_l(const AutoUTF &chrs) {
	wstring::size_type	pos	= this->find_first_not_of(chrs);
	if (pos && (pos != wstring::npos)) {
		this->erase(0, pos);
	}
	return	*this;
}
AutoUTF&			AutoUTF::Trim_r(const AutoUTF &chrs) {
	wstring::size_type	pos	= this->find_last_not_of(chrs);
	if (pos && (pos != string::npos) && (++pos < this->size())) {
		this->erase(pos);
	}
	return	*this;
}

bool				AutoUTF::AsNum(size_t &num, int base) const {
	PWSTR	end_ptr;
	num = ::wcstoll(m_str.c_str(), &end_ptr, base);
	return	end_ptr != m_str.c_str();
}
bool				AutoUTF::AsNum(ssize_t &num, int base) const {
	PWSTR	end_ptr;
	num = ::wcstoull(m_str.c_str(), &end_ptr, base);
	return	end_ptr != m_str.c_str();
}

///================================================================================= String Utilites
string					StrUtil::CutWord(string &inout, CONSTR &delim, bool dd) {
	string	Result;
	size_t	pos = inout.find_first_of(delim);
	Result = inout.substr(0, pos);
	if (dd && pos != string::npos)
		pos = inout.find_first_not_of(delim, pos);
	inout.erase(0, pos);
	return	Result;
}
wstring					StrUtil::CutWord(wstring &inout, CONSTRW &delim, bool dd) {
	wstring		Result;
	wstring::size_type pos = inout.find_first_of(delim);
	Result = inout.substr(0, pos);
	if (dd && pos != wstring::npos)
		pos = inout.find_first_not_of(delim, pos);
	inout.erase(0, pos);
	return	Result;
}
string					StrUtil::CutWordEx(string &inout, CONSTR &delim, bool dd) {
	string	Result;
	size_t	pos = inout.find(delim);
	Result = inout.substr(0, pos);
	if (dd && pos != string::npos)
		pos += delim.size();
	inout.erase(0, pos);
	return	Result;
}
wstring					StrUtil::CutWordEx(wstring &inout, CONSTRW &delim, bool dd) {
	wstring		Result;
	wstring::size_type pos = inout.find(delim);
	Result = inout.substr(0, pos);
	if (dd && pos != wstring::npos)
		pos += delim.size();
	inout.erase(0, pos);
	return	Result;
}
string&					StrUtil::AddWord(string &inout, CONSTR &add, CONSTR &delim) {
	size_t	pos = inout.size() - delim.size();
	if (!(delim.empty() || inout.empty() || (inout.rfind(delim) == pos) || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return	inout;
}
wstring&				StrUtil::AddWord(wstring &inout, CONSTRW &add, CONSTRW &delim) {
	size_t	pos = inout.size() - delim.size();
	if (!(delim.empty() || inout.empty() || (inout.rfind(delim) == pos) || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return	inout;
}

string&					StrUtil::AddWordEx(string &inout, CONSTR &add, CONSTR &delim) {
	size_t	pos = inout.size() - delim.size();
	if (!(add.empty() || delim.empty() || inout.empty() || (inout.rfind(delim) == pos) || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return	inout;
}
wstring&				StrUtil::AddWordEx(wstring &inout, CONSTRW &add, CONSTRW &delim) {
	size_t	pos = inout.size() - delim.size();
	if (!(add.empty() || delim.empty() || inout.empty() || (inout.rfind(delim) == pos) || (add.find(delim) == 0)))
		inout += delim;
	if (!add.empty())
		inout += add;
	return	inout;
}

///============================================================================================= Str
AutoUTF					ErrAsStr(DWORD err, PCWSTR lib) {
	AutoUTF	Result;
	PWSTR	buf = NULL;
	HMODULE	mod = NULL;
	if (err != 0 && lib) {
		mod = ::LoadLibraryW(lib);
	}
	::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | ((mod) ?  FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM),
		mod,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PWSTR)&buf, 0, NULL);
	if (err)
		Result = (buf) ? AutoUTF(L"ERROR (") + n2w(err) + L"): " + buf : L"Unknown error\r\n";
	else
		Result = (buf) ? wstring(buf) : L"Unknown error\r\n";
	Result.resize(Result.size() - 2);
	::LocalFree(buf);
	if (mod)
		::FreeLibrary(mod);
	return	Result;
}

AutoUTF					AsStr(const SYSTEMTIME &in, bool tolocal) {
	SYSTEMTIME	stTime;
	if (tolocal) {
		::SystemTimeToTzSpecificLocalTime(NULL, (SYSTEMTIME*)&in, &stTime);
	} else {
		stTime = in;
	}
	WCHAR	buf[MAX_PATH];
	_snwprintf(buf, sizeofa(buf), L"%04d-%02d-%02d %02d:%02d:%02d",
			   stTime.wYear, stTime.wMonth, stTime.wDay,
			   stTime.wHour, stTime.wMinute, stTime.wSecond);
	return	buf;
}
AutoUTF					AsStr(const FILETIME &in) {
	SYSTEMTIME	stUTC;
	::FileTimeToSystemTime(&in, &stUTC);
	return	AsStr(stUTC);
}

AutoUTF					AsStr(size_t num, int base) {
	AutoUTF	tmp;
	return	tmp;
}
AutoUTF					AsStr(ssize_t &num, int base) {
	AutoUTF	tmp;
	return	tmp;
}

AutoUTF					CopyAfterLast(const AutoUTF &in, const AutoUTF &delim) {
	AutoUTF	Result;
	wstring::size_type pos = in.find_last_of(delim);
	if (pos != string::npos) {
		Result = in.substr(pos + 1);
	}
	return	Result;
}

AutoUTF&				Cut(AutoUTF &inout, const AutoUTF &in) {
	wstring::size_type pos = inout.find(in);
	if (pos != wstring::npos) {
		inout.erase(pos, in.size());
	}
	return	inout;
}
bool					Cut(AutoUTF &inout, ssize_t &num, int base) {
	return	inout.Cut(num, base);
}
AutoUTF&				CutAfter(AutoUTF &inout, const AutoUTF &delim) {
	wstring::size_type pos = inout.find_first_of(delim);
	if (pos != wstring::npos) {
		inout.erase(pos);
	}
	return	inout;
}
AutoUTF&				CutBefore(AutoUTF &inout, const AutoUTF &delim) {
	size_t	pos = inout.find_first_of(delim);
	if (pos != 0) {
		inout.erase(0, pos);
	}
	return	inout;
}

AutoUTF&				ToLower(AutoUTF &inout) {
	if (!inout.empty())
		::CharLowerW((WCHAR*)inout.c_str());
	return	inout;
}
AutoUTF					ToLowerOut(const AutoUTF &in) {
	AutoUTF	tmp(in);
	return	ToLower(tmp);
}
AutoUTF&				ToUpper(AutoUTF &inout) {
	if (!inout.empty())
		::CharUpperW((WCHAR*)inout.c_str());
	return	inout;
}
AutoUTF					ToUpperOut(const AutoUTF &in) {
	AutoUTF	tmp(in);
	return	ToUpper(tmp);
}

/*
bool					Hash2Str(string &out, const char data[], size_t size) {
	out.clear();
	out.reserve(size);
	char tmp[3] = {0};
	for (size_t i = 0; i < size; ++i) {
		_snprintf(tmp, sizeofa(tmp), "%02x", (unsigned char)data[i]);
		out += tmp;
	}
	return	true;
}
bool					Str2Hash(const string &str, PVOID &hash, ULONG &size) {
	size_t strsize = str.size();
	if (strsize % 2 == 0) {
		size = strsize / 2;
		if (WinMem::Alloc(hash, size)) {
			for (size_t i = 0; i < size; ++i) {
				string	tmp = str.substr(i * 2, 2);
				((PBYTE)hash)[i] = (BYTE)s2l(tmp, 16);
			}
			return	true;
		}
	}
	return	false;
}
*/
#endif
