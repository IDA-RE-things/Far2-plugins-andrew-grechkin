#ifndef _LIBCOM_STD_HPP_
#define _LIBCOM_STD_HPP_

#include <libbase/std.hpp>

namespace Com {

	void init();

	HRESULT ConvertBoolToHRESULT(bool result);

	///=================================================================================== ComObject
	template<typename Interface>
	class ComObject {
		typedef ComObject this_type;
		typedef Interface * pointer;

	public:
		~ComObject()
		{
			Release();
		}

		ComObject() :
			m_obj(nullptr)
		{
		}

		explicit ComObject(const Interface * param) :
			m_obj((pointer)param)
		{ // caller must not Release param
		}

		explicit ComObject(const VARIANT & param) :
			m_obj((pointer)param.ppunkVal)
		{
			m_obj->AddRef();
		}

		ComObject(const this_type & param) :
			m_obj(param.m_obj)
		{
			if (m_obj) {
				m_obj->AddRef();
			}
		}

		this_type & operator =(const pointer rhs)
		{ // caller must not Release rhs
			if (m_obj != rhs) {
				this_type tmp(rhs);
				swap(tmp);
			}
			return *this;
		}
		this_type & operator =(const this_type & rhs)
		{
			if (m_obj != rhs.m_obj) {
				this_type tmp(rhs);
				swap(tmp);
			}
			return *this;
		}

		void Release()
		{
			if (m_obj) {
				m_obj->Release();
				m_obj = nullptr;
			}
		}

		operator bool() const
		{
			return m_obj;
		}
		operator pointer() const
		{
			return m_obj;
		}

		pointer * operator &()
		{
			Release();
			return &m_obj;
		}
		pointer operator ->() const
		{
			return m_obj;
		}

		bool operator ==(const pointer rhs) const
		{
			return m_obj == rhs;
		}
		bool operator ==(const this_type & rhs) const
		{
			return m_obj == rhs.m_obj;
		}
		bool operator !=(const pointer rhs) const
		{
			return m_obj != rhs;
		}
		bool operator !=(const this_type & rhs) const
		{
			return m_obj != rhs.m_obj;
		}

		void attach(pointer & param)
		{
			Release();
			m_obj = param;
			param = nullptr;
		}
		void detach(pointer & param)
		{
			param = m_obj;
			m_obj = nullptr;
		}

		void swap(this_type & rhs)
		{
			using std::swap;
			swap(m_obj, rhs.m_obj);
		}

	private:
		pointer m_obj;
	};

}

#endif
