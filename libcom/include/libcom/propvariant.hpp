#ifndef _LIBCOM_PROPVARIANT_HPP_
#define _LIBCOM_PROPVARIANT_HPP_

#include <libbase/std.hpp>


namespace Com {

	class PropVariant: public PROPVARIANT {
		typedef PROPVARIANT base_type;
		typedef PropVariant this_type;
		typedef PROPVARIANT * pointer;

	public:
		~PropVariant();

		PropVariant();
		PropVariant(PCWSTR val);
		PropVariant(const ustring & val);
		PropVariant(bool val);
		PropVariant(uint32_t val);
		PropVariant(uint64_t val);
		PropVariant(const FILETIME & val);

		PropVariant(const base_type & var);
		this_type & operator =(const base_type & rhs);

		this_type & operator =(PCWSTR rhs);
		this_type & operator =(const ustring & rhs);
		this_type & operator =(bool rhs);
		this_type & operator =(uint32_t rhs);
		this_type & operator =(uint64_t rhs);
		this_type & operator =(const FILETIME & rhs);

		bool is_empty() const {
			return vt == VT_EMPTY;
		}
		bool is_null() const {
			return vt == VT_NULL;
		}
		bool is_bool() const {
			return vt == VT_BOOL;
		}
		bool is_int() const {
			return vt == VT_I1 || vt == VT_I2 || vt == VT_I4 || vt == VT_INT || vt == VT_I8;
		}
		bool is_uint() const {
			return vt == VT_UI1 || vt == VT_UI2 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI8;
		}
		bool is_str() const {
			return vt == VT_BSTR || vt == VT_LPWSTR;
		}
		bool is_time() const {
			return vt == VT_FILETIME;
		}

		size_t get_int_size() const;

		HRESULT as_bool_nt(bool & val) const throw();
		HRESULT as_str_nt(ustring & val) const throw();

		bool as_bool() const;
		FILETIME as_time() const;
		ustring	as_str() const;
		int64_t	as_int() const;
		uint64_t as_uint() const;

		pointer ref();

		void detach(pointer var);

		void swap(this_type & rhs);
	};

}


#endif
