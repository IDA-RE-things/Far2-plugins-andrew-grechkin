/**
	win_autostr
	@author		© 2010 Andrew Grechkin
	@link ()
**/
#ifndef WIN_AUTOSTR_HPP
#define WIN_AUTOSTR_HPP

template<typename Type, typename From>
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
		m_data(Alloc(1)) {
	}

	AutoSTR(size_type len, Type in):
		m_data(Alloc(len) + 1) {
		Init(in, len);
	}

	AutoSTR(const Type * in, size_t len = 0):
		m_data(Alloc(((in && len == 0) ? Len(in) : len) + 1)) {
		Init(in, len);
	}

	AutoSTR(const this_type & in):
		m_data(in.m_data) {
		addRef();
	}

	AutoSTR(const From * in, UINT cp = DEFAULT_CP):
		m_data(Alloc(Convert(in, cp))) {
		m_data->m_size = Convert(in, cp, buffer(), capacity()) - 1;
	}

	size_t capacity() const {
		return m_data->m_capa;
	}
	void clear() {
		split();
		m_data->m_size = 0;
		m_data->m_str1[0] = 0;
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
	this_type & cp(const From * in, UINT cp = CP_UTF8) {
		reserve(convert(in, cp));
		convert(in, cp, buffer(), capacity());
		return *this;
	}
	AutoSTR<From, Type>	utf8(const Type * tst = nullptr) const {
		return AutoSTR<From, Type>(c_str(), CP_UTF8);
	}
	AutoSTR<From, Type>	utf16(const Type * tst = nullptr) const {
		return AutoSTR<From, Type>(c_str(), CP_UTF16le);
	}

	const Type * c_str() const {
		return m_data->m_str1;
	}
	Type * buffer() {
		split();
		return m_data->m_str1;
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
			WinMem::Copy(m_data->m_str1 + size(), s, n * sizeof(Type));
			m_data->m_size += n;
			m_data->m_str1[size()] = (Type)0;
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
			WinMem::Copy(m_data->m_str1, s, n * sizeof(Type));
			m_data->m_size = n;
			m_data->m_str1[size()] = (Type)0;
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
			tmp.append(m_data->m_str1, pos);
			tmp.append(str);
			if ((pos + n1) < size())
				tmp.append(&m_data->m_str1[pos + n1]);
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
				this_type(&m_data->m_str1[pos])
			:
				this_type(&m_data->m_str1[pos], n);
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
		return m_data->m_str1[in];
	}
	const Type & operator [](int in) const {
		return m_data->m_str1[in];
	}
	Type & at(size_t in) {
		split();
		return m_data->m_str1[in];
	}
	const Type & at(size_t in) const {
		return m_data->m_str1[in];
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
	struct	Cont {
		size_t	m_capa;
		size_t	m_ref;
		size_t	m_size;
		Type	m_str1[1];

		Cont(): m_capa(1), m_ref(1), m_size(0) {
			m_str1[0] = 0;
		}
	} *m_data;

	void delRef() {
		if (m_data && --m_data->m_ref == 0) {
			WinMem::Free(m_data);
		}
	}

	void addRef() {
		++m_data->m_ref;
	}

	void Init(Type in, size_t len) {
		m_data->m_size = len;
		if (in != (Type)0) {
			WinMem::Fill(m_data->m_str1, len * sizeof(Type), in);
		}
	}

	void Init(const Type * in, size_t len) {
		m_data->m_size = len;
		WinMem::Copy(m_data->m_str1, in, len * sizeof(Type));
	}

	Cont * Alloc(size_t capa) {
		Cont * ret = (Cont *)WinMem::Alloc(sizeof(Cont) + capa * sizeof(Type));
		ret->m_ref = 1;
		ret->m_capa = capa;
		return ret;
	}

	void split() {
		if (m_data->m_ref > 1)
			this_type(capacity(), this).swap(*this);
	}

	AutoSTR(size_t capa, const AutoSTR * str): m_data(Alloc(capa)) {
		if (str)
			Init(str->c_str(), str->size());
	}

	AutoSTR(const Type * str1, size_t size1, const Type * str2, size_t size2): m_data(Alloc(size1 + size2 + 1)) {
		if (size1) {
			WinMem::Copy(m_data->m_str1, str1, size1 * sizeof(Type));
			m_data->m_size = size1;
		}
		if (size2) {
			WinMem::Copy(m_data->m_str1 + size1, str2, size2 * sizeof(Type));
			m_data->m_size += size2;
		}
	}

	template<typename Type1, typename From1>
	friend AutoSTR<Type1, From1> operator +(const Type1 * lhs, const AutoSTR<Type1, From1> & rhs);
};

template<typename Type, typename From>
inline AutoSTR<Type, From> operator+(const Type *lhs, const AutoSTR<Type, From> &rhs) {
	return AutoSTR<Type, From>(lhs, Len(lhs), rhs.c_str(), rhs.size());
}

typedef AutoSTR<CHAR, WCHAR> astring;
typedef AutoSTR<WCHAR, CHAR> ustring;

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

#endif // WIN_AUTOSTR_HPP
