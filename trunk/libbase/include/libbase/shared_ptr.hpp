#ifndef _LIBBASE_SHARED_PTR_HPP_
#define _LIBBASE_SHARED_PTR_HPP_

#include <libbase/std.hpp>

namespace Base {

	template<typename Type>
	class shared_ptr {
		typedef Type element_type;
		typedef shared_ptr this_type;

	public:
		typedef size_t size_type;

		~shared_ptr() throw()
		{
			reset();
		}

		shared_ptr() :
			m_impl(nullptr)
		{
		}

		explicit shared_ptr(element_type * ptr) :
			m_impl(new shared_ptr_impl(ptr))
		{
		}

		template<typename Deleter>
		shared_ptr(element_type * ptr, Deleter d) :
			m_impl(new shared_ptr_impl_deleter<Deleter>(ptr, d))
		{
		}

		shared_ptr(const this_type & sh_ptr) :
			m_impl(nullptr)
		{
			if (sh_ptr.m_impl) {
				m_impl = sh_ptr.m_impl;
				m_impl->inc_ref();
			}
		}

		this_type & operator =(const this_type & sh_ptr)
		{
			if (m_impl != sh_ptr.m_impl) {
				shared_ptr(sh_ptr).swap(*this);
			}
			return *this;
		}

		template<typename newType>
		operator shared_ptr<newType>()
		{
			return shared_ptr<newType>(m_impl);
		}

		void reset()
		{
			if (m_impl) {
				m_impl->dec_ref();
				m_impl = nullptr;
			}
		}

		void reset(element_type * p)
		{
			shared_ptr(p).swap(*this);
		}

		template<typename Deleter>
		void reset(element_type * p, Deleter d)
		{
			shared_ptr(p, d).swap(*this);
		}

		element_type & operator *() const
		{
			return *(m_impl->get());
		}

		element_type * operator ->() const
		{
			return m_impl->get();
		}

		element_type * get() const
		{
			return (m_impl) ? m_impl->get() : nullptr;
		}

		bool unique() const
		{
			return !m_impl || m_impl->refcnt() == 1;
		}

		size_type use_count() const
		{
			return (m_impl) ? m_impl->refcnt() : 0;
		}

		operator bool() const
		{
			return m_impl && m_impl->get();
		}

		void swap(this_type & b) throw()
		{
			using std::swap;
			swap(m_impl, b.m_impl);
		}

	private:
		struct shared_ptr_impl {
			virtual ~shared_ptr_impl()
			{
			}

			shared_ptr_impl(element_type * ptr) :
				m_ptr(ptr),
				m_refcnt(1)
			{
			}

			virtual void del_ptr()
			{
				delete m_ptr;
			}

			void inc_ref()
			{
				m_refcnt++;
			}

			void dec_ref()
			{
				if (--m_refcnt == 0) {
					del_ptr();
					delete this;
				}
			}

			size_type refcnt() const
			{
				return m_refcnt;
			}

			element_type * get() const
			{
				return m_ptr;
			}

		protected:
			element_type * m_ptr;
			size_type m_refcnt;
		};

		template<typename Deleter>
		struct shared_ptr_impl_deleter: public shared_ptr_impl {
			shared_ptr_impl_deleter(element_type * ptr, Deleter d) :
				shared_ptr_impl(ptr),
				m_deleter(d)
			{
			}

			void del_ptr()
			{
				m_deleter(this->m_ptr);
			}

		private:
			Deleter m_deleter;
		};

		shared_ptr_impl * m_impl;
	};

	template<class T, class U>
	bool operator ==(const shared_ptr<T> & a, const shared_ptr<U> & b)
	{
		return a.get() == b.get();
	}

	template<class T, class U>
	bool operator !=(const shared_ptr<T> & a, const shared_ptr<U> & b)
	{
		return a.get() != b.get();
	}

	template<class T, class U>
	bool operator <(const shared_ptr<T> & a, const shared_ptr<U> & b)
	{
		return a.get() < b.get();
	}

	template<class T>
	void swap(shared_ptr<T> & a, shared_ptr<T> & b) throw()
	{
		a.swap(b);
	}

}

#endif
