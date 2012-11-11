#include <libcom/bstr.hpp>
#include <libext/exception.hpp>


namespace Com {

	BStr::BStr(PCWSTR val):
		m_str(nullptr)
	{
		if (val) {
			m_str = ::SysAllocString(val);
			if (!m_str)
				CheckCom(E_OUTOFMEMORY);
		}
	}

	BStr::BStr(const ustring & val)
	{
		m_str = ::SysAllocStringLen(val.c_str(), val.size());
		if (!m_str)
			CheckCom(E_OUTOFMEMORY);
	}

	BStr::BStr(const this_type & val):
		m_str(nullptr)
	{
		if (val.m_str) {
			m_str = ::SysAllocStringLen(val.m_str, val.size());
			if (!m_str)
				CheckCom(E_OUTOFMEMORY);
		}
	}

	BStr& BStr::operator=(PCWSTR val) {
		if (!::SysReAllocString(&m_str, val))
			CheckCom(E_OUTOFMEMORY);
		return *this;
	}

	BStr& BStr::operator=(const ustring& val) {
		if (!::SysReAllocStringLen(&m_str, val.c_str(), val.size()))
			CheckCom(E_OUTOFMEMORY);
		return *this;
	}

	BStr& BStr::operator=(const this_type & val) {
		if (this != &val) {
			if (!::SysReAllocStringLen(&m_str, val.m_str, val.size()))
				CheckCom(E_OUTOFMEMORY);
		}
		return *this;
	}

	size_t BStr::size() const {
		if (!m_str)
			CheckCom(E_POINTER);
		return ::SysStringLen(m_str);
	}

	BSTR* BStr::operator&() {
		clean();
		return &m_str;
	}

	void BStr::attach(BSTR & str) {
		clean();
		m_str = str;
		str = nullptr;
	}

	void BStr::detach(BSTR & str) {
		str = m_str;
		m_str = nullptr;
	}

	void BStr::swap(this_type & rhs) {
		using std::swap;
		swap(m_str, rhs.m_str);
	}

	void BStr::clean() {
		if (m_str) {
			::SysFreeString(m_str);
			m_str = nullptr;
		}
	}

}
