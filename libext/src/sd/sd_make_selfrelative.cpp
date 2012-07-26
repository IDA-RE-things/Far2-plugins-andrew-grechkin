#include <libext/sd.hpp>
#include <libext/exception.hpp>

namespace Ext {

	WinSD::WinSD(PSECURITY_DESCRIPTOR sd) {
		DWORD size = WinSD::size(sd);
		m_sd = alloc(size);
		CheckApi(::MakeSelfRelativeSD(sd, m_sd, &size));
		//TODO may be leak
	}

}
