#ifndef _WIN_DEF_REF_CNT_H_
#define _WIN_DEF_REF_CNT_H_

#include "std.h"

namespace winstd {
	class ref_counter {
	public:
		virtual ~ref_counter() {
		}

		ref_counter():
			m_cnt(0),
			m_shareable(true) {
		}
		ref_counter(const ref_counter &rhs):
			m_cnt(0),
			m_shareable(true) {
		}

		ref_counter& operator=(const ref_counter &rhs) {
			return *this;
		}

		void add_ref() {
			++m_cnt;
		}

		void del_ref() {
			if (--m_cnt == 0)
				delete this;
		}

		void mark_unshareable() {
			m_shareable = false;
		}

		bool is_shareable() const {
			return m_shareable;
		}

		bool is_shared() const {
			return m_cnt > 1;
		}

	private:
		size_t m_cnt;
		bool m_shareable;
	};

	template<class Type>
	class rc_ptr {
	public:
		~rc_ptr() {
			if (m_ptr)
				m_ptr->del_ref();
		}
		rc_ptr(Type* ptr = nullptr):
			m_ptr(ptr) {
			init();
		}
		rc_ptr(const rc_ptr &rhs):
			m_ptr(rhs.m_ptr) {
			init();
		}
		rc_ptr& operator=(const rc_ptr &rhs) {
			if (m_ptr != rhs.m_ptr) {
				if (m_ptr)
					m_ptr->del_ref();
				m_ptr = rhs.m_ptr;
				init();
			}
			return *this;
		}

		Type* operator->() const {
			return m_ptr;
		}
		Type& operator*() const {
			return *m_ptr;
		}
	private:
		void init() {
			if (!m_ptr)
				return;
			if (!m_ptr->is_shareable())
				m_ptr = new Type(*m_ptr);
			m_ptr->add_ref();
		}
		Type* m_ptr;
	};
}

#endif
