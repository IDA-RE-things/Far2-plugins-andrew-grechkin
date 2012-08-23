#include <libext/service.hpp>
#include <libext/exception.hpp>

namespace Ext {

	ServiceStop::ServiceStop(const ustring & name, RemoteConnection * conn):
		m_name(name),
		m_conn(conn)
	{
	}

	size_t ServiceStop::execute() {
		WinSvc::Stop(m_name, m_conn);
		return 0;
	}

}
