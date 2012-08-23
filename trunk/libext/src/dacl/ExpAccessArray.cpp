#include <libext/dacl.hpp>
#include <libext/exception.hpp>

namespace Ext {

	///============================================================================== ExpAccessArray
	ExpAccessArray::~ExpAccessArray() {
		::LocalFree(m_eacc);
	}

	ExpAccessArray::ExpAccessArray(size_t count_ea):
		m_eacc((ExpAccess*)CheckPointer(::LocalAlloc(LPTR, sizeof(ExpAccess) * count_ea))),
		m_size(count_ea)
	{
	}

	ExpAccessArray::ExpAccessArray(PACL acl) {
		CheckApiError(::GetExplicitEntriesFromAclW(acl, &m_size, (PEXPLICIT_ACCESSW*)&m_eacc));
	}

	ExpAccess & ExpAccessArray::operator [] (size_t index) const {
		return m_eacc[index];
	}

	size_t ExpAccessArray::size() const {
		return m_size;
	}


}
