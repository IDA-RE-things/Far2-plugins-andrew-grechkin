#ifndef _LIBBASE_MSTRING_HPP_
#define _LIBBASE_MSTRING_HPP_

#include <libbase/std.hpp>

///========================================================================================= mstring
namespace Base {

	struct mstring: public Uncopyable {
		~mstring();

		mstring(PCWSTR in = EMPTY_STR);

		mstring(mstring && rhs);

		mstring & operator = (mstring && rhs);

		void push_back(PCWSTR str);

		size_t size() const;

		size_t capacity() const;

		PCWSTR c_str() const;

		PCWSTR operator [] (size_t index) const;

	private:
		struct impl;

		impl * m_str;
	};

}

#endif
