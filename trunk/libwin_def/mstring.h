#ifndef WIN_DEF_MSTRING_HPP
#define WIN_DEF_MSTRING_HPP

#include "std.h"
#include "shared_ptr.h"

///========================================================================================= mstring
struct mstring {
	mstring(PCWSTR in = L"");

	size_t size() const;

	size_t capacity() const;

	PCWSTR c_str() const;

	PCWSTR operator [](int index) const;

private:
	struct impl {
		~impl();

		explicit impl(PCWSTR in);

	private:
		PWSTR m_data;
		size_t m_capa;
		size_t m_size;

		friend class mstring;
	};

	winstd::shared_ptr<impl> m_str;
};

#endif
