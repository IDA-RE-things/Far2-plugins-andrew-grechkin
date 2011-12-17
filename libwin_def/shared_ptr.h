#ifndef WIN_SHARED_PTR_H
#define WIN_SHARED_PTR_H

#include <stdint.h>
#include <algorithm>

namespace winstd {
	template <typename Type>
	struct shared_ptr {
		typedef Type element_type;

		shared_ptr():
			m_impl(nullptr) {
		}

		explicit shared_ptr(Type* ptr):
			m_impl(new shared_ptr_impl(ptr)) {
		}

		template< typename Deleter>
		shared_ptr(Type* ptr, Deleter d):
			m_impl(new shared_ptr_impl_deleter<Deleter>(ptr, d)) {
		}

		shared_ptr(const shared_ptr& sh_ptr):
			m_impl(nullptr) {
			if (sh_ptr.m_impl) {
				m_impl = sh_ptr.m_impl;
				m_impl->inc_ref();
			}
		}

		shared_ptr& operator=(const shared_ptr& sh_ptr) {
			if (m_impl != sh_ptr.m_impl) {
				shared_ptr(sh_ptr).swap(*this);
			}
			return *this;
		}

		~shared_ptr() throw() {
			reset();
		}

		template<typename newType>
		operator shared_ptr<newType>() {
			return shared_ptr<newType>(m_impl);
		}

		void reset() {
			if (m_impl) {
				m_impl->dec_ref();
				m_impl = nullptr;
			}
		}

		void reset(Type* p) {
			shared_ptr(p).swap(*this);
		}

		template<typename Deleter>
		void reset(Type* p, Deleter d) {
			shared_ptr(p, d).swap(*this);
		}

		Type& operator* () const {
			return *(m_impl->get());
		}

		Type* operator-> () const {
			return m_impl->get();
		}

		Type* get() const {
			return (m_impl) ? m_impl->get() : nullptr;
		}

		bool unique() const {
			return !m_impl || m_impl->refcnt() == 1;
		}

		size_t use_count() const {
			return (m_impl) ? m_impl->refcnt() : 0;
		}

		operator bool() const {
			return m_impl && m_impl->get();
		}

		void swap(shared_ptr &b) {
			using std::swap;
			swap(m_impl, b.m_impl);
		}

	private:
		struct shared_ptr_impl {
			shared_ptr_impl(Type * ptr):
				m_ptr(ptr),
				m_refcnt(1) {
			}
			virtual ~shared_ptr_impl() {
			}
			virtual void del_ptr() {
				delete m_ptr;
			}

			void inc_ref() {
				m_refcnt++;
			}
			void dec_ref() {
				if (--m_refcnt == 0) {
					del_ptr();
					delete this;
				}
			}
			size_t refcnt() const {
				return m_refcnt;
			}
			Type * get() const {
				return m_ptr;
			}

		protected:
			Type * m_ptr;
			size_t m_refcnt;
		};

		template <typename Deleter>
		struct shared_ptr_impl_deleter : public shared_ptr_impl {
			shared_ptr_impl_deleter(Type * ptr, Deleter d):
				shared_ptr_impl(ptr),
				m_deleter(d) {
			}
			void del_ptr() {
				m_deleter(this->m_ptr);
			}
		private:
			Deleter m_deleter;
		};

		shared_ptr_impl * m_impl;
	};

	template<class T, class U>
	bool operator==(const shared_ptr<T> &a, const shared_ptr<U> &b) {
		return a.get() == b.get();
	}

	template<class T, class U>
	bool operator!=(const shared_ptr<T> &a, const shared_ptr<U> & b) {
		return a.get() != b.get();
	}

	template<class T, class U>
	bool operator<(const shared_ptr<T> &a, const shared_ptr<U> &b) {
		return a.get() < b.get();
	}

	template<class Type>
	void swap(shared_ptr<Type> &a, shared_ptr<Type> &b) {
		a.swap(b);
	}
}

#endif
