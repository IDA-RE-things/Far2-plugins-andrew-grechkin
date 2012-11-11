#ifndef _LIBCOM_SAFEARRAY_HPP_
#define _LIBCOM_SAFEARRAY_HPP_

#include <libcom/variant.hpp>


namespace Com {

	struct SafeArray {
		~SafeArray();

		SafeArray(VARTYPE type, size_t size);
		SafeArray(SAFEARRAY * ptr);
		SafeArray(const Variant & var);

		size_t dims() const;

		size_t size() const;

		template<typename Type>
		Type& at(size_t index) const {
			return *(((Type*)m_ptr->pvData) + index);
		}
		//	size_t array_size(size_t dim = 1) const {
		//		LONG lbound, ubound;
		//		CheckCom(::SafeArrayGetLBound(parray, dim, &lbound));
		//		CheckCom(::SafeArrayGetUBound(parray, dim, &ubound));
		//		return ubound - lbound + 1;
		//	}
		//	void at(size_t index, BSTR &out) const {
		//		LONG lbound;
		//		CheckCom(::SafeArrayGetLBound(parray, 1, &lbound));
		//		lbound += index;
		//		CheckCom(::SafeArrayGetElement(parray, &lbound, &out));
		//	}

		operator SAFEARRAY *() const {
			return m_ptr;
		}

	private:
		SAFEARRAY * m_ptr;
	};

}


#endif
