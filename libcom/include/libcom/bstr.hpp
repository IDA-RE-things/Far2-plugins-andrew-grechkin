#ifndef _LIBCOM_BSTR_HPP_
#define _LIBCOM_BSTR_HPP_

#include <libbase/std.hpp>

namespace Com {

	class BStr {
		typedef BStr this_type;
		typedef this_type * pointer;

	public:
		~BStr()
		{
			clean();
		}

		BStr() :
			m_str(nullptr)
		{
		}

		BStr(PCWSTR val);

		BStr(const ustring & val);

		BStr(const this_type & val);

		this_type & operator =(PCWSTR val);

		this_type & operator =(const ustring & val);

		this_type & operator =(const this_type & val);

		size_t size() const;

		BSTR * operator &();

		operator BSTR() const
		{
			return m_str;
		}

		operator PCWSTR() const
		{
			return m_str;
		}

		operator bool() const
		{
			return m_str;
		}

		PCWSTR c_str() const
		{
			return m_str;
		}

		void attach(BSTR & str);

		void detach(BSTR & str);

		void swap(this_type & rhs);

	private:
		void clean();

		BSTR m_str;
	};

}

#endif
