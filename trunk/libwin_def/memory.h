#ifndef _WIN_DEF_MEMORY_HPP
#define _WIN_DEF_MEMORY_HPP

#include "std.h"

#ifdef NoStdNew
inline void * operator new(size_t size) {
	return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

inline void * operator new [](size_t size) {
	return ::operator new(size);
}

inline void operator delete(void * in) throw() {
	::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
}

inline void operator delete [](void * ptr) throw() {
	::operator delete(ptr);
}
#endif

///========================================================================================== WinMem
/// Функции работы с кучей
namespace WinMem {
	template<typename Type>
	inline bool Alloc(Type & in, size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		in = static_cast<Type> (::HeapAlloc(::GetProcessHeap(), flags, size));
		return in != nullptr;
	}

	inline PVOID Alloc(size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		return ::HeapAlloc(::GetProcessHeap(), flags, size);
	}

	inline PVOID Realloc_(PVOID in, size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		if (in)
			in = ::HeapReAlloc(::GetProcessHeap(), flags, in, size);
		else
			in = ::HeapAlloc(::GetProcessHeap(), flags, size);
		return in;
	}

	template<typename Type>
	inline bool Realloc(Type & in, size_t size, DWORD flags = HEAP_ZERO_MEMORY) {
		in = (Type)Realloc_(in, size, flags);
		return in != nullptr;
	}

	inline void Free_(PVOID in) {
		if (in) {
			::HeapFree(::GetProcessHeap(), 0, in);
		}
	}

	template<typename Type>
	inline void Free(Type & in) {
		Free_((PVOID)in);
		in = nullptr;
	}

	inline size_t Size(PCVOID in) {
		return (in) ? ::HeapSize(::GetProcessHeap(), 0, in) : 0;
	}

	inline bool Cmp(PCVOID m1, PCVOID m2, size_t size) {
		return ::memcmp(m1, m2, size) == 0;
	}

	inline PVOID Copy(PVOID dest, PCVOID sour, size_t size) {
		return ::memcpy(dest, sour, size);
	}

	inline PVOID Fill(PVOID in, size_t size, char fill) {
		return ::memset(in, (int)fill, size);
	}

	inline void Zero(PVOID in, size_t size) {
		Fill(in, size, 0);
	}

	template<typename Type>
	inline void Fill(Type &in, char fill) {
		Fill(&in, sizeof(in), fill);
	}

	template<typename Type>
	inline void Zero(Type &in) {
		Fill(&in, sizeof(in), 0);
	}
}

template<typename Type>
inline Type & reverse_bytes(Type & inout) {
	std::reverse((char*)&inout, ((char*)&inout) + sizeof(inout));
	return inout;
}

inline WORD & swap_bytes(WORD & inout) {
	inout = inout >> 8 | inout << 8;
	return inout;
}

inline DWORD & swap_words(DWORD &inout) {
	inout = inout >> 16 | inout << 16;
	return inout;
}

inline intmax_t Mega2Bytes(size_t in) {
	return (in != 0) ? (intmax_t)in << 20 : -1ll;
}

inline size_t Bytes2Mega(intmax_t in) {
	return (in > 0) ? in >> 20 : 0;
}

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ constraints
template <typename Type>
struct must_be_pointer {
	static bool constraints(const Type & type_is_not_pointer) {
		return sizeof(0[type_is_not_pointer]);
	}
};

template <>
struct must_be_pointer<PVOID> {
	static bool constraints(const PVOID &) {
		return true;
	}
};

///======================================================================================== auto_buf
template<typename Type>
struct auto_buf {
	typedef auto_buf<Type> this_type;
	typedef Type value_type;
	typedef size_t size_type;

	~auto_buf() {
		must_be_pointer<Type>::constraints(m_ptr);
		WinMem::Free(m_ptr);
	}

	auto_buf():
		m_ptr(nullptr) {
	}

	explicit auto_buf(size_type size):
		m_ptr((value_type)WinMem::Alloc(size, 0)) {
	}

	auto_buf(const this_type & rhs): // move semantic
		m_ptr(nullptr) {
		swap((this_type&)rhs);
	}

	this_type & operator =(const this_type & rhs) {
		if (this != &rhs)
			this_type(rhs).swap(*this);
		return *this;
	}

	void reserve(size_type nsize) {
		if (size() < nsize) {
			WinMem::Realloc(m_ptr, nsize);
		}
	}

	size_type size() const {
		return WinMem::Size(m_ptr);
	}

	value_type operator &() const {
		return m_ptr;
	}

	value_type operator ->() const {
		return m_ptr;
	}

	operator value_type() const {
		return m_ptr;
	}

	value_type data() const {
		return m_ptr;
	}

	bool operator !() const {
		return m_ptr;
	}

	void attach(value_type & ptr) {
		WinMem::Free(m_ptr);
		m_ptr = ptr;
	}

	void detach(value_type & ptr) {
		ptr = m_ptr;
		m_ptr = nullptr;
	}

	void swap(value_type & ptr) throw() {
		using std::swap;
		swap(m_ptr, ptr);
	}

	void swap(this_type & rhs) throw() {
		using std::swap;
		swap(m_ptr, rhs.m_ptr);
	}

private:
	value_type m_ptr;
};

template<typename Type>
void swap(auto_buf<Type> & b1, auto_buf<Type> & b2) {
	b1.swap(b2);
}

///======================================================================================== auto_buf
template<typename Type>
class auto_array {
	typedef auto_array<Type> this_type;
	typedef Type value_type;
	typedef Type * pointer_type;
	typedef size_t size_type;

public:
	~auto_array() {
		WinMem::Free(m_ptr);
	}

	explicit auto_array(size_type size):
		m_ptr((pointer_type)WinMem::Alloc(size * sizeof(Type), 0)),
		m_size(size) {
	}

	auto_array(const this_type & rhs):
		m_ptr(nullptr),
		m_size(0) {
		swap((this_type&)rhs);
	}

	this_type & operator =(const this_type & rhs) {
		if (this != &rhs)
			this_type(rhs).swap(*this);
		return *this;
	}

	void reserve(size_type nsize) {
		if (size() < nsize) {
			WinMem::Realloc(m_ptr, nsize * sizeof(Type));
			m_size = nsize;
		}
	}

	size_type size() const {
		return m_size;
	}

	operator pointer_type() const {
		return m_ptr;
	}

	pointer_type data() const {
		return m_ptr;
	}

	value_type & operator [](int ind) {
		return m_ptr[ind];
	}

	const value_type & operator [](int ind) const {
		return m_ptr[ind];
	}

	void detach(pointer_type & ptr, size_t & size) {
		ptr = m_ptr;
		size = m_size;
		m_ptr = nullptr;
		m_size = 0;
	}

	void swap(this_type & rhs) {
		using std::swap;
		swap(m_ptr, rhs.m_ptr);
		swap(m_size, rhs.m_size);
	}

private:
	pointer_type m_ptr;
	size_type m_size;
};

template<typename Type>
void swap(auto_array<Type> & b1, auto_array<Type> & b2) {
	b1.swap(b2);
}

///====================================================================================== auto_close
template<typename Type>
class auto_close: private Uncopyable {
	typedef auto_close<Type> this_type;
	typedef Type value_type;

public:
	~auto_close() {
		delete m_impl;
	}

	template<typename Deleter>
	explicit auto_close(value_type ptr, Deleter del) :
		m_impl(new auto_close_deleter_impl<Deleter>(ptr, del)) {
	}

	operator value_type() {
		return m_impl->m_ptr;
	}

	value_type * operator &() {
		return &(m_impl->m_ptr);
	}

	void swap(this_type & rhs) {
		using std::swap;
		swap(m_impl, rhs.m_impl);
	}

private:
	struct auto_close_impl {
		auto_close_impl(value_type ptr):
			m_ptr(ptr) {
		}
		virtual ~auto_close_impl() {
		}
		value_type m_ptr;
	};

	template <typename Deleter>
	struct auto_close_deleter_impl: public auto_close_impl {
		auto_close_deleter_impl(value_type ptr, Deleter d):
			auto_close_impl(ptr),
			m_deleter(d) {
		}
		virtual ~auto_close_deleter_impl() {
			m_deleter(this->m_ptr);
		}
		Deleter m_deleter;
	};

	auto_close_impl * m_impl;
};

template<typename Type>
inline void swap(auto_close<Type> & b1, auto_close<Type> & b2) {
	b1.swap(b2);
}

template<>
struct auto_close<HANDLE>: private Uncopyable {
	typedef HANDLE value_type;
	typedef auto_close<value_type> this_type;

	~auto_close() {
		close();
	}

	explicit auto_close(value_type ptr = nullptr):
		m_ptr(ptr) {
	}

	value_type * operator &() {
		close();
		return &m_ptr;
	}

	operator value_type() const {
		return m_ptr;
	}

	operator bool() const {
		return m_ptr && m_ptr != INVALID_HANDLE_VALUE;
	}

	void close();

	void swap(this_type & rhs) {
		using std::swap;
		swap(m_ptr, rhs.m_ptr);
	}

private:
	value_type m_ptr;
};

inline void swap(auto_close<HANDLE> & b1, auto_close<HANDLE> & b2) {
	b1.swap(b2);
}

#endif
