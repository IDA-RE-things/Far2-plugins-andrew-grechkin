#ifndef WIN_SHARED_PTR_H
#define WIN_SHARED_PTR_H

#include <stdint.h>
#include <algorithm>

namespace winstd {
	template <typename Type>
	class shared_ptr {
	public:
		typedef Type element_type;

		shared_ptr():
			m_impl(NULL) {
		}

		explicit shared_ptr( Type* ptr ):
			m_impl(new shared_ptr_impl(ptr)) {
		}

		template< typename Deleter>
		shared_ptr( Type* ptr, Deleter d ):
			m_impl(new shared_ptr_impl_deleter<Deleter>(ptr, d)) {
		}

		shared_ptr(const shared_ptr& sh_ptr):
			m_impl(NULL) {
			if (sh_ptr.m_impl) {
				m_impl = sh_ptr.m_impl;
				m_impl->inc_ref();
			}
		}

		shared_ptr& operator= (const shared_ptr& sh_ptr) {
			if (this != &sh_ptr) {
				shared_ptr tmp(sh_ptr);
				swap(tmp);
			}
			return *this;
		}

		~shared_ptr() {
			reset();
		}

		void reset() {
			if (m_impl) {
				m_impl->dec_ref();
				m_impl = NULL;
			}
		}

		void reset(Type* p) {
			shared_ptr tmp(p);
			swap(tmp);
		}

		template< typename Deleter>
		void reset(Type* p, Deleter d) {
			shared_ptr tmp(p, d);
			swap(tmp);
		}

		Type& operator* () const {
			return *(m_impl->get());
		}

		Type* operator-> () const {
			return m_impl->get();
		}

		Type* get() const {
			return m_impl->get();
		}

		bool unique() const {
			return m_impl->refcnt() == 1;
		}

		size_t use_count() const {
			return m_impl->refcnt();
		}

		void swap(shared_ptr &b) {
			using std::swap;
			swap(m_impl, b.m_impl);
		}

		operator bool() const {
			return m_impl && m_impl->get();
		}

	private:
		class shared_ptr_impl {
		public:
			shared_ptr_impl(Type* ptr):
				ptr_(ptr),
				refcnt_(1) {
			}

			virtual ~shared_ptr_impl() {
			}

			virtual void del_ptr() {
				delete ptr_;
			}

			void inc_ref() {
				refcnt_++;
			}

			void dec_ref() {
				if (--refcnt_ == 0) {
					if (ptr_) {
						del_ptr();
					}
					delete this;
				}
			}

			size_t refcnt() const {
				return refcnt_;
			}

			Type* get() const {
				return ptr_;
			}

		protected:
			Type* ptr_;
			size_t refcnt_;
		};

		template <typename Deleter>
		class shared_ptr_impl_deleter : public shared_ptr_impl {
		public:
			shared_ptr_impl_deleter(Type* ptr, Deleter d):
				shared_ptr_impl(ptr), m_deleter(d) {
			}
			void del_ptr() {
				m_deleter(this->ptr_);
			}
		private:
			Deleter m_deleter;
		};

		shared_ptr_impl *m_impl;
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
