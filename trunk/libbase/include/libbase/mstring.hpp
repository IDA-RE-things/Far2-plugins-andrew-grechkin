#ifndef _LIBBASE_MSTRING_HPP_
#define _LIBBASE_MSTRING_HPP_

#include <libbase/std.hpp>
#include <libbase/shared_ptr.hpp>

///========================================================================================= mstring
namespace Base {

	struct mstring {
		mstring(PCWSTR in = L"");

		size_t size() const;

		size_t capacity() const;

		PCWSTR c_str() const;

		PCWSTR operator [](size_t index) const;

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

		shared_ptr<impl> m_str;
	};

}

#endif
