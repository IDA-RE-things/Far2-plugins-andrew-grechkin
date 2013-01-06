#ifndef _LIBCOM_VARIANT_HPP_
#define _LIBCOM_VARIANT_HPP_

#include <libbase/std.hpp>

namespace Com {

	class Variant: public VARIANT {
		typedef VARIANT base_type;
		typedef Variant this_type;
		typedef this_type * pointer;

	public:
		~Variant();
		Variant();
		Variant(IUnknown * val);
		Variant(PCWSTR val);
		Variant(PCWSTR val[], size_t cnt);
		Variant(size_t val[], size_t cnt, VARTYPE type);
		Variant(const ustring &val);
		Variant(const ustring val[], size_t cnt);
		Variant(bool val);
		Variant(DWORD in);
		Variant(int64_t in);
		Variant(uint64_t in);
		Variant(uint16_t in);

		Variant(const base_type & in);
		const this_type & operator =(const base_type & in);

		bool is_empty() const
		{
			return vt == VT_EMPTY;
		}
		bool is_null() const
		{
			return vt == VT_NULL;
		}
		bool is_bool() const
		{
			return vt == VT_BOOL;
		}
		bool is_int() const
		{
			return vt == VT_I1 || vt == VT_I2 || vt == VT_I4 || vt == VT_INT || vt == VT_I8;
		}
		bool is_uint() const
		{
			return vt == VT_UI1 || vt == VT_UI2 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI8;
		}
		bool is_str() const
		{
			return vt == VT_BSTR;
		}
		bool is_array() const
		{
			return (vt & VT_ARRAY) == VT_ARRAY;
		}

		VARTYPE Type() const
		{
			return vt;
		}

		void change_type(DWORD type, DWORD flag = 0);

		HRESULT change_type_nt(VARTYPE type, DWORD flag = 0) throw();

		bool as_bool() const;
		int64_t as_int() const;
		uint64_t as_uint() const;
		ustring as_str() const;
		ustring as_str();

		pointer ref();

		void swap(this_type & rhs);
	};

}

#endif
