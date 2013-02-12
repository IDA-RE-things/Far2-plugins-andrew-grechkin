#ifndef _LIBBASE_AUTOUTF_HPP_
#define _LIBBASE_AUTOUTF_HPP_

#include <libbase/std.hpp>

#ifdef NoStlString
#include <libbase/memory.hpp>
template<typename Type>
class AutoSTR {
	typedef AutoSTR this_type;
	typedef Type char_type;

public:
	typedef size_t size_type;

	static const size_type npos = ~(size_type)0;

	~AutoSTR()
	{
		delRef();
	}

	AutoSTR() :
		m_data(alloc_cstr(1))
	{
	}

	AutoSTR(size_type len, char_type in):
		m_data(alloc_cstr(len + 1))
	{
		init(in, len);
	}

	AutoSTR(const char_type * in, size_t len = 0) :
		m_data(alloc_cstr(((in && len == 0) ? (len = Base::Str::length(in)) : len) + 1))
	{
		init(in, len);
	}

	AutoSTR(const this_type & in) :
		m_data(in.m_data)
	{
		addRef();
	}

	size_t capacity() const
	{
		return m_data->m_capa;
	}
	void clear()
	{
		split();
		m_data->m_size = 0;
		m_data->m_str[0] = 0;
	}
	bool empty() const
	{
		return m_data->m_size == 0;
	}
	void reserve(size_t capa)
	{
		if (capacity() < capa) {
			if (m_data->m_ref > 1) {
				this_type(capa, this).swap(*this);
			} else {
				Base::Memory::realloc(m_data, sizeof(*m_data) + capa * sizeof(Type));
				m_data->m_capa = capa;
			}
		}
	}
	size_t size() const
	{
		return m_data->m_size;
	}

	const Type * c_str() const
	{
		return m_data->m_str;
	}
	Type * buffer()
	{
		split();
		return m_data->m_str;
	}
	void swap(this_type & in)
	{
		using std::swap;
		swap(m_data, in.m_data);
	}

	this_type & append(const this_type & str)
	{
		if (m_data != str.m_data) {
			append(str.c_str(), str.size());
		} else {
			this_type tmp(size() + str.size(), this);
			tmp.append(str.c_str(), str.size());
			swap(tmp);
		}
		return *this;
	}
	this_type & append(const this_type & str, size_t pos, size_t n = npos)
	{
		return append(&str[pos], n);
	}
	this_type & append(const Type * s, size_t n)
	{
		if (n) {
			reserve(size() + n);
			Base::Memory::copy(m_data->m_str + size(), s, n * sizeof(Type));
			m_data->m_size += n;
			m_data->m_str[size()] = (Type)0;
		}
		return *this;
	}
	this_type & append(const Type *s)
	{
		return append(s, Base::Str::length(s));
	}
	this_type & append(size_t n, Type c)
	{
		return append(this_type(n, c));
	}

	this_type & assign(const this_type & in)
	{
		if (this != &in) {
			delRef();
			m_data = in.m_data;
			addRef();
		}
		return *this;
	}
	this_type & assign(const this_type & str, size_t pos, size_t n = npos)
	{
		return assign(&str[pos], n);
	}
	this_type & assign(const Type * s, size_t n)
	{
		if (n) {
			reserve(n);
			Base::Memory::copy(m_data->m_str, s, n * sizeof(Type));
			m_data->m_size = n;
			m_data->m_str[size()] = (Type)0;
		}
		return *this;
	}
	this_type & assign(const Type * s)
	{
		return assign(s, Base::Str::length(s));
	}
	this_type & assign(size_t n, Type c)
	{
		return assign(this_type(n, c));
	}

	this_type & replace(size_t pos, size_t n1, const this_type & str)
	{
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
	this_type & erase(size_t pos = 0, size_t n = npos)
	{
		if (pos < size()) {
			size_t size2 = n == npos ? size() : std::min(size(), pos + n);
			this_type(c_str(), pos, c_str() + size2, size() - size2).swap(*this);
		}
		return *this;
	}
	this_type substr(size_t pos = 0, size_t n = npos) const
	{
		if (pos < size()) {
			return (n == npos) ? this_type(&m_data->m_str[pos]) : this_type(&m_data->m_str[pos], n);
		}
		return *this;
	}

	const this_type & operator =(const this_type & in)
	{
		return assign(in);
	}
	this_type & operator +=(const this_type & in)
	{
		return append(in);
	}
	this_type & operator +=(const Type * in)
	{
		return append(in);
	}
	this_type & operator +=(Type in)
	{
		Type tmp[] = {in, 0};
		return append(tmp, 1);
	}

	this_type operator +(const this_type & in) const
	{
		this_type tmp(size() + in.size(), this);
		return tmp += in;
	}
	this_type operator +(const Type * in) const
	{
		this_type tmp(size() + Base::Str::length(in), this);
		return tmp += in;
	}
	this_type operator +(Type in) const
	{
		this_type tmp(size() + 1);
		return tmp += in;
	}

	bool operator ==(const this_type & in) const
	{
		return Base::Str::compare(c_str(), in.c_str()) == 0;
	}
	bool operator ==(const Type * in) const
	{
		return Base::Str::compare(c_str(), in) == 0;
	}
	bool operator!=(const this_type & in) const
	{
		return !operator ==(in);
	}
	bool operator !=(const Type * in) const
	{
		return !operator ==(in);
	}

	bool operator <(const this_type & in) const
	{
		return Base::Str::compare(c_str(), in.c_str()) < 0;
	}
	bool operator <(const Type * in) const
	{
		return Base::Str::compare(c_str(), in) < 0;
	}
	bool operator >(const this_type & in) const
	{
		return Base::Str::compare(c_str(), in.c_str()) > 0;
	}
	bool operator >(const Type * in) const
	{
		return Base::Str::compare(c_str(), in) > 0;
	}
	bool operator <=(const this_type & in) const
	{
		return operator ==(in) || operator <(in);
	}
	bool operator <=(const Type * in) const
	{
		return operator==(in) || operator <(in);
	}
	bool operator >=(const this_type & in) const
	{
		return operator ==(in) || operator >(in);
	}
	bool operator >=(const Type * in) const
	{
		return operator ==(in) || operator >(in);
	}

	Type & operator [](int in)
	{
		split();
		return m_data->m_str[in];
	}
	const Type & operator [](int in) const
	{
		return m_data->m_str[in];
	}
	Type & at(size_t in)
	{
		split();
		return m_data->m_str[in];
	}
	const Type & at(size_t in) const
	{
		return m_data->m_str[in];
	}

	size_t find(Type c, size_t p = 0) const
	{
		Type what[] = {c, 0};
		return find(what, p);
	}
	size_t find(const this_type & in, size_t p = 0) const
	{
		return find(in.c_str(), p);
	}
	size_t find(const Type * s, size_t p = 0) const
	{
		const Type * pos = Base::Str::find(c_str() + std::min(p, size()), s);
		if (pos) {
			return pos - c_str();
		}
		return npos;
	}
	size_t rfind(const this_type & /*in*/) const
	{
//		const Type * pos = RFind(c_str(), in.c_str());
//		if (pos)
//			return pos - c_str();
		return npos;
	}

	size_t find_first_of(const this_type & str, size_t pos = 0) const
	{
		size_t Result = Base::Str::span(c_str() + pos, str.c_str());
		return (Result < size()) ? Result : npos;
	}
	size_t find_last_of(const this_type & str, size_t pos = npos) const
	{
		size_t Result = Base::Str::span(c_str() + pos, str.c_str());
		return (Result < size()) ? Result : npos;
	}
	size_t find_first_not_of(const this_type & str, size_t pos = 0) const
	{
		for (; pos < size(); ++pos)
			if (Base::Str::find(str.c_str(), at(pos)))
				return pos;
		return npos;
	}
	size_t find_last_not_of(const this_type & str, size_t pos = npos) const
	{
		size_t __size = size();
		if (__size) {
			if (--__size > pos)
				__size = pos;
			do {
				if (!Base::Str::find(str.c_str(), at(__size)))
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
		size_t m_ref;
		size_t m_capa;
		size_t m_size;
		Type m_str[1];

		Cont() :
			m_ref(1), m_capa(1), m_size(0)
		{
			m_str[0] = 0;
		}
	}* m_data;

	void delRef()
	{
		if (m_data && --m_data->m_ref == 0) {
			Base::Memory::free(m_data);
		}
	}

	void addRef()
	{
		++m_data->m_ref;
	}

	void init(Type in, size_t len)
	{
		if (in != (Type)0) {
			std::fill(m_data->m_str, &m_data->m_str[len], in);
		}
		m_data->m_size = len;
		m_data->m_str[len] = (Type)0;
	}

	void init(const Type * in, size_t len)
	{
		Base::Memory::copy(m_data->m_str, in, len * sizeof(Type));
		m_data->m_size = len;
		m_data->m_str[len] = (Type)0;
	}

	Cont * alloc_cstr(size_t capa)
	{
		Cont * ret = (Cont *)Base::Memory::alloc(sizeof(Cont) + capa * sizeof(Type));
		ret->m_ref = 1;
		ret->m_capa = capa;
		return ret;
	}

	void split()
	{
		if (m_data->m_ref > 1)
			this_type(capacity(), this).swap(*this);
	}

	AutoSTR(size_t capa, const AutoSTR * str) :
		m_data(alloc_cstr(capa))
	{
		if (str)
			init(str->c_str(), str->size());
	}

	AutoSTR(const Type * str1, size_t size1, const Type * str2, size_t size2) :
		m_data(alloc_cstr(size1 + size2 + 1))
	{
		if (size1) {
			Base::Memory::copy(m_data->m_str, str1, size1 * sizeof(Type));
			m_data->m_size = size1;
		}
		if (size2) {
			Base::Memory::copy(m_data->m_str + size1, str2, size2 * sizeof(Type));
			m_data->m_size += size2;
		}
	}

	template<typename Type1>
	friend AutoSTR<Type1> operator +(const Type1 * lhs, const AutoSTR<Type1> & rhs);
};

template<typename Type>
inline AutoSTR<Type> operator +(const Type * lhs, const AutoSTR<Type> & rhs)
{
	return AutoSTR<Type>(lhs, Base::Str::length(lhs), rhs.c_str(), rhs.size());
}

typedef AutoSTR<char> astring;
typedef AutoSTR<wchar_t> ustring;

#else

#include <string>

typedef std::string astring;
typedef std::wstring ustring;

#endif

namespace Base {

	astring w2cp(PCWSTR in, UINT cp);

	ustring cp2w(PCSTR in, UINT cp);

}

#endif
