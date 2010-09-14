/**
	win_autostr
	@author		© 2010 Andrew Grechkin
	@link ()
**/
#ifndef WIN_AUTOSTR_HPP
#define WIN_AUTOSTR_HPP

template<typename Type, typename From>
class		AutoSTR {
protected:
	struct	Cont {
		size_t		m_ref;
		size_t		m_capa;
		size_t		m_size;
		Type		m_str;

		Cont(): m_ref(1), m_capa(1), m_size(0), m_str(0) {
		}
	}*	m_data;

	void	delRef() {
		if (m_data && --m_data->m_ref == 0) {
			WinMem::Free(m_data);
		}
	}
	void	addRef() {
		++m_data->m_ref;
	}

	void	Init(Cont* data, Type in, size_t len) {
		data->m_size = len;
		if (in != (Type)0) {
			for (size_t i = 0; i < len; ++i) {
				*(&data->m_str + i) = in;
			}
		}
	}
	void	Init(Cont* data, const Type *in, size_t len) {
		data->m_size = len;
		WinMem::Copy(&data->m_str, in, (len + 1) * sizeof(Type));
	}
	void	Alloc(Cont* &data, size_t capa) {
		WinMem::Alloc(data, sizeof(*data) + capa * sizeof(Type));
		data->m_ref = 1;
		data->m_capa = capa;
		data->m_size = 0;
	}
public:
	static const	size_t	npos = (size_t) - 1;
	~AutoSTR() {
		delRef();
	}
	AutoSTR(): m_data(NULL) {
		Alloc(m_data, 1);
	}
	AutoSTR(int capa): m_data(NULL) {
		Alloc(m_data, capa);
	}
	AutoSTR(size_t len, Type in): m_data(NULL) {
		Alloc(m_data, len + 1);
		Init(m_data, in, len);
	}
	AutoSTR(const Type *in, size_t len = 0): m_data(NULL) {
		if (in && len == 0)
			len = Len(in);
		Alloc(m_data, len + 1);
		Init(m_data, in, len);
	}
	AutoSTR(const AutoSTR &in): m_data(NULL) {
		m_data = in.m_data;
		addRef();
	}
	AutoSTR(const From* in, UINT cp): m_data(NULL) {
		Alloc(m_data, Convert(in, cp));
		m_data->m_size = Convert(in, cp, buffer(), capacity()) - 1;
	}

	AutoSTR&		cp(const From* in, UINT cp = CP_UTF8) {
		reserve(convert(in, cp));
		convert(in, cp, buffer(), capacity());
		return	*this;
	}

	size_t			capacity() const {
		return	m_data->m_capa;
	}
	void			clear() {
		split();
		zero();
	}
	bool			empty() const {
		return	m_data->m_str == (Type)0;
	}
	void			reserve(size_t capa) {
		if (m_data->m_capa < capa) {
			split();
			WinMem::Realloc(m_data, sizeof(*m_data) + capa * sizeof(Type));
			m_data->m_capa = capa;
		}
	}
	size_t			size() const {
		return	m_data->m_size;
	}
	void			split() {
		if (m_data->m_ref > 1) {
			Cont*	tmp = NULL;
			Alloc(tmp, capacity());
			Init(tmp, c_str(), size());
			delRef();
			m_data = tmp;
		}
	}
	AutoSTR<From, Type>	utf8(const Type *tst = NULL) const {
		AutoSTR<From, Type>	Result(c_str(), CP_UTF8);
		return	Result;
	}
	AutoSTR<From, Type>	utf16(const Type *tst = NULL) const {
		AutoSTR<From, Type>	Result(c_str(), CP_UTF16le);
		return	Result;
	}
	void			zero() {
		WinMem::Zero(&m_data->m_str, (m_data->m_capa + 1) * sizeof(Type));
	}

	operator		const Type*() const {
		return	&m_data->m_str;
	}
	const Type*		c_str() const {
		return	&m_data->m_str;
	}
	Type*			buffer() {
		split();
		return	&m_data->m_str;
	}
	void			swap(AutoSTR& in) {
		Swp(m_data, in.m_data);
	}

	AutoSTR&		append(const AutoSTR& str) {
		append(str, str.size());
		return	*this;
	}
	AutoSTR&		append(const AutoSTR& str, size_t pos, size_t n = npos) {
		append(&str[pos], n);
		return	*this;
	}
	AutoSTR&		append(PCWSTR s, size_t n) {
		if (n == npos)
			n = Len(s);
		reserve(size() + n + 1);
		WinMem::Copy(&m_data->m_str + size(), s, n * sizeof(Type));
		m_data->m_size += n;
		*(&m_data->m_str + size()) = (Type)0;
		return	*this;
	}
	AutoSTR&		append(PCWSTR s) {
		append(s, Len(s));
		return	*this;
	}
	AutoSTR&		append(size_t n, Type c) {
		AutoSTR	tmp(*this);
		AutoSTR	add(n, c);
		tmp += add;
		swap(tmp);
		return	*this;
	}
	AutoSTR& 		replace(size_t pos1, size_t n1, const AutoSTR& str) {
		AutoSTR	tmp(*this, pos1);
		tmp.reserve(size() - n1 + str.size() + 1);
		tmp += str;
		tmp.append(&m_data->m_str + (pos1 + n1), size() - n1 + pos1);
		swap(tmp);
		return	*this;
	}
	AutoSTR&		erase(size_t pos = 0, size_t n = npos) {
//		m_str.erase(pos, n);
		return	*this;
	}
	AutoSTR			substr(size_t pos = 0, size_t n = npos) const {
		if (n == npos) {
			return	AutoSTR(&m_data->m_str + pos);
		}
		return	AutoSTR(&m_data->m_str + pos, n);
	}

	const AutoSTR&	operator=(const AutoSTR &in) {
		if (this != &in) {
			delRef();
			m_data = in.m_data;
			addRef();
		}
		return	*this;
	}
	AutoSTR&		operator+=(const AutoSTR &in) {
		if (m_data != in.m_data) {
			operator+=(in.c_str());
		}
		return	*this;
	}
	AutoSTR&		operator+=(const Type *in) {
		split();
		size_t	sz = Len(in);
		reserve(size() + sz + 1);
		Cat(&m_data->m_str, in);
		m_data->m_size += sz;
		return	*this;
	}
	AutoSTR&		operator+=(Type in) {
		Type tmp[] = {in, 0};
		operator+=(tmp);
		return	*this;
	}

	AutoSTR			operator+(const AutoSTR &in) const {
		AutoSTR	tmp(*this);
		return	tmp += in;
	}
	AutoSTR			operator+(const Type *in) const {
		AutoSTR	tmp(*this);
		return	tmp += in;
	}
	AutoSTR			operator+(Type in) const {
		AutoSTR	tmp(*this);
		return	tmp += in;
	}

	bool			operator==(const AutoSTR &in) const {
		return	Eq(c_str(), in.c_str());
	}
	bool			operator==(const Type *in) const {
		return	Eq(c_str(), in);
	}
	bool			operator!=(const AutoSTR &in) const {
		return	!operator==(in);
	}
	bool			operator!=(const Type *in) const {
		return	!operator==(in);
	}

	bool			operator<(const AutoSTR &in) const {
		return	Cmp(c_str(), in.c_str()) < 0;
	}
	bool			operator<(const Type *in) const {
		return	Cmp(c_str(), in) < 0;
	}
	bool			operator>(const AutoSTR &in) const {
		return	Cmp(c_str(), in.c_str()) > 0;
	}
	bool			operator>(const Type *in) const {
		return	Cmp(c_str(), in) > 0;
	}
	bool			operator<=(const AutoSTR &in) const {
		return	operator==(in) || operator<(in);
	}
	bool			operator<=(const Type *in) const {
		return	operator==(in) || operator<(in);
	}
	bool			operator>=(const AutoSTR &in) const {
		return	operator==(in) || operator>(in);
	}
	bool			operator>=(const Type *in) const {
		return	operator==(in) || operator>(in);
	}

	Type&			operator[](int in) {
		split();
		return	*(&m_data->m_str + in);
	}
	const Type&		operator[](int in) const {
		return	*(&m_data->m_str + in);
	}
	Type&			at(size_t in) {
		split();
		return	*(&m_data->m_str + in);
	}
	const Type&		at(size_t in) const {
		return	*(&m_data->m_str + in);
	}
	size_t			find(Type c, size_t p = 0) const {
		Type	what[] = {c, 0};
		return	find(what, p);
	}
	size_t			find(const AutoSTR &in, size_t p = 0) const {
		return	find(in.c_str(), p);
	}
	size_t			find(const Type *s, size_t p = 0) const {
		PCWSTR	pos = ::Find(c_str() + Min(p, size()), s);
		if (pos) {
			return	pos - c_str();
		}
		return	npos;
	}

	size_t			rfind(const AutoSTR &in) const {
		PCWSTR	pos = RFind(c_str(), in.c_str());
		if (pos)
			return	pos - c_str();
		return	npos;
	}

	size_t			find_first_of(const AutoSTR &str, size_t pos = 0) const {
		size_t	Result = ::wcscspn(c_str() + pos, str.c_str());
		return	(Result < size()) ? Result : npos;
	}
	size_t			find_last_of(const AutoSTR &str, size_t pos = npos) const {
		size_t	Result = ::wcsspn(c_str() + pos, str.c_str());
		return	(Result < size()) ? Result : npos;
	}
	size_t			find_first_not_of(const AutoSTR &str, size_t pos = 0) const {
		for (; pos < size(); ++pos)
			if (::Find(str.c_str(), at(pos)))
				return	pos;
		return	npos;
	}
	size_t			find_last_not_of(const AutoSTR &str, size_t pos = npos) const {
		size_t	__size = size();
		if (__size) {
			if (--__size > pos)
				__size = pos;
			do {
				if (!Find(str.c_str(), at[__size]))
					return __size;
			} while (__size--);
		}
		return	npos;
	}

	bool			Find(wchar_t c, size_t pos = 0) const {
		return	find(c, pos) != npos;
	}
	bool			Find(const AutoSTR &sub) const {
		return	find(sub) != npos;
	}
	bool			Find(const AutoSTR &sub, size_t &pos) const {
		pos = find(sub);
		return	pos != npos;
	}
	AutoSTR&		Add(const Type add)  {
		size_t	pos = this->size() - 1;
		if (!(empty() || (at(pos) == add)))
			operator+=(add);
		return	*this;
	}
	AutoSTR&		Add(const AutoSTR &add)  {
		size_t	pos = this->size() - add.size();
		if (!(add.empty() || empty() || (rfind(add) == pos)))
			this->operator+=(add);
		return	*this;
	}
	AutoSTR&		Add(const AutoSTR &add, const AutoSTR &delim, bool chkEmpty = true) {
		size_t	pos = size() - delim.size();
		if (!(add.empty() || delim.empty() || (chkEmpty && empty()) || (rfind(delim) == pos) || (add.find(delim) == 0)))
			operator+=(delim);
		if (!add.empty())
			operator+=(add);
		return	*this;
	}
	AutoSTR&		Cut(const AutoSTR &sub) {
		size_t	pos;
		if (Find(sub, pos)) {
			erase(pos, sub.size());
		}
		return	*this;
	}
	bool			Cut(ssize_t &num, int base = 10) {
		size_t	pos1 = find_first_of(L"0123456789");
		if (pos1 == npos)
			return	false;
		size_t	pos2 = find_first_not_of(L"0123456789", pos1);
		if (pos1 > 0 && at(pos1 - 1) == L'-')
			--pos1;
		AutoSTR	tmp(substr(pos1, pos2));
		tmp.AsNum(num, base);
		erase(0, pos2);
		return	true;
	}
	AutoSTR			CutWord(const AutoSTR &delim = L"\t ", bool delDelim = true) {
		size_t	pos = find(delim);
		AutoSTR		Result(substr(0, pos));
		if (delDelim && pos != npos)
			pos += delim.size();
		erase(0, pos);
		return	Result;
	}

	bool			AsNum(size_t &num, int base = 10) const {
		PWSTR	end_ptr;
		num = ::wcstoll(c_str(), &end_ptr, base);
		return	end_ptr != c_str();
	}
	bool			AsNum(ssize_t &num, int base = 10) const {
		PWSTR	end_ptr;
		num = ::wcstoull(c_str(), &end_ptr, base);
		return	end_ptr != c_str();
	}
};

typedef		AutoSTR<CHAR, WCHAR>	CStrA;
typedef		AutoSTR<WCHAR, CHAR>	AutoUTF;

inline CStrA			w2cp(PCWSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	CHAR	buf[size];
	Convert(in, cp, buf, size);
	return	buf;
}
inline AutoUTF			cp2w(PCSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	WCHAR	buf[size];
	Convert(in, cp, buf, size);
	return	buf;
}

inline AutoUTF			operator+(PCWSTR lhs, const AutoUTF &rhs) {
	AutoUTF	tmp(lhs);
	return	tmp += rhs;
}

#endif // WIN_AUTOSTR_HPP
