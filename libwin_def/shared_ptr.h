#ifndef WIN_SHARED_PTR_H
#define WIN_SHARED_PTR_H

#include <stdint.h>
#include <algorithm>

namespace winstd {
	template <typename Type>
	class shared_ptr {
	public:
		typedef Type element_type;

		shared_ptr(): impl_(NULL) {}

		explicit
		shared_ptr( Type* ptr ): impl_(new shared_ptr_impl(ptr)) {}

		template< typename Deleter >
		shared_ptr( Type* ptr, Deleter d ): impl_(new shared_ptr_impl_d<Deleter>(ptr, d)) {}

		shared_ptr(const shared_ptr& sh_ptr): impl_(NULL) {
			if (sh_ptr.impl_) {
				impl_ = sh_ptr.impl_;
				impl_->inc_ref();
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
			if (impl_) {
				impl_->dec_ref();
				impl_ = NULL;
			}
		}

		void reset(Type* p) {
			shared_ptr tmp(p);
			swap(tmp);
		}

		template< typename Deleter >
		void reset(Type* p, Deleter d) {
			shared_ptr tmp(p, d);
			swap(tmp);
		}

		Type& operator* () const {
			return *(impl_->get());
		}

		Type* operator-> () const {
			return impl_->get();
		}

		Type* get() const {
			return impl_->get();
		}

		bool unique() const {
			return impl_->refcnt() == 1;
		}

		size_t use_count() const {
			return impl_->refcnt();
		}

		void swap(shared_ptr &b) {
			using std::swap;
			swap(impl_, b.impl_);
		}

		operator bool() const {
			return impl_ && impl_->get();
		}

	private:

		class shared_ptr_impl {
		public:
			shared_ptr_impl(Type* ptr): ptr_(ptr), refcnt_(1) {}
			virtual ~shared_ptr_impl() {}

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
		class shared_ptr_impl_d : public shared_ptr_impl {
		public:
			shared_ptr_impl_d(Type* ptr, Deleter d): shared_ptr_impl(ptr), d_(d) {}
			void del_ptr() {
				d_(this->ptr_);
			}
		private:
			Deleter d_;
		};

		shared_ptr_impl *impl_;

	public:
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
