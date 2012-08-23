#include <libext/sd.hpp>
#include <libext/exception.hpp>

namespace Ext {

	WinSD::WinSD(const trustee_t & owner, const trustee_t & group, const ExpAccessArray * dacl, const ExpAccessArray * sacl) {
		ULONG size = 0;
		LogTrace();
		CheckApiError(
			::BuildSecurityDescriptorW(
			(PTRUSTEEW)&owner,
			(PTRUSTEEW)&group,
			dacl ? dacl->size() : 0,
			(PEXPLICIT_ACCESS_W)dacl ? &(*dacl)[0] : nullptr,
			sacl ? sacl->size() : 0,
			(PEXPLICIT_ACCESS_W)sacl ? &(*sacl)[0] : nullptr,
			nullptr,
			&size,
			&m_sd
		));
	}

	WinSD::WinSD(PSECURITY_DESCRIPTOR sd) {
		DWORD size = WinSD::size(sd);
		m_sd = alloc(size);
		CheckApi(::MakeSelfRelativeSD(sd, m_sd, &size));
		//TODO may be leak
	}

}
