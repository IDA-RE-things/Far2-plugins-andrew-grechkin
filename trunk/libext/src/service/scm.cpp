#include <libext/service.hpp>
#include <libext/exception.hpp>
#include <libext/rc.hpp>

#include <libbase/logger.hpp>


namespace Ext {

	///===================================================================================== Manager
	Service::Manager::~Manager() {
		LogTrace();
		close(m_hndl);
	}

	Service::Manager::Manager(RemoteConnection * conn, ACCESS_MASK acc):
		m_hndl(open(conn, acc))
	{
		LogTrace();
	}

	Service::Manager::Manager(Manager && right):
			m_hndl(right.m_hndl)
	{
		right.m_hndl = nullptr;
	}

	Service::Manager & Service::Manager::operator = (Manager && right) {
		m_hndl = right.m_hndl;
		right.m_hndl = nullptr;
		return *this;
	}

	void Service::Manager::reconnect(RemoteConnection * conn, ACCESS_MASK acc) {
		SC_HANDLE l_hndl = open(conn, acc);
		std::swap(m_hndl, l_hndl);
		close(l_hndl);
	}

	Service Service::Manager::create_service(const Service::Create_t & info) const {
		SC_HANDLE hndl = CheckHandleErr(
			::CreateServiceW(m_hndl, info.name.c_str(),
			                 info.displayName,
			                 SERVICE_ALL_ACCESS,
			                 info.serviceType,
			                 info.startType,
			                 info.errorControl,
			                 info.binaryPathName.c_str(),
			                 info.loadOrderGroup,
			                 info.tagId,
			                 info.dependencies,
			                 nullptr,
			                 nullptr)
			                 );
		return std::move(Service(hndl));
	}
	//	Service & Service::Create(const ustring & /*name*/, const ustring & path, DWORD /*StartType*/, PCWSTR /*dispname*/) {
	//		WCHAR fullpath[MAX_PATH_LEN];
	//		if (path.empty()) {
	//			CheckApi(::GetModuleFileNameW(0, fullpath, sizeofa(fullpath)));
	//		} else {
	//			copy_str(fullpath, path.c_str(), sizeofa(fullpath));
	//		}
	//		Manager(nullptr, SC_MANAGER_CREATE_SERVICE).create_service(name.c_str(), fullpath, StartType, dispname);
	//	}


	bool Service::Manager::is_exist(PCWSTR name) const {
		SC_HANDLE hndl = ::OpenServiceW(m_hndl, name, SERVICE_QUERY_STATUS);
		if (hndl)
			::CloseServiceHandle(hndl);
		return hndl;
	}

	SC_HANDLE Service::Manager::open(RemoteConnection * conn, ACCESS_MASK acc) {
		return CheckHandleErr(::OpenSCManagerW(conn->get_host().c_str(), nullptr, acc));
	}

	void Service::Manager::close(SC_HANDLE scm) {
		if (scm)
			::CloseServiceHandle(scm);
	}

}
