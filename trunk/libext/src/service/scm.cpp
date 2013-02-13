#include <libext/service.hpp>
#include <libext/exception.hpp>
#include <libext/rc.hpp>

#include <libbase/logger.hpp>

namespace Ext {

	///===================================================================================== Manager
	Service::Manager::~Manager()
	{
		close(m_hndl);
		LogTrace();
	}

	Service::Manager::Manager(RemoteConnection * conn, ACCESS_MASK acc) :
		m_hndl(open(conn, acc))
	{
		LogTrace();
	}

	Service::Manager::Manager(Manager && right):
		m_hndl(right.m_hndl)
	{
		right.m_hndl = nullptr;
	}

	Service::Manager & Service::Manager::operator =(Manager && right)
	{
		m_hndl = right.m_hndl;
		right.m_hndl = nullptr;
		return *this;
	}

	void Service::Manager::reconnect(RemoteConnection * conn, ACCESS_MASK acc)
	{
		LogTrace();
		SC_HANDLE l_hndl = open(conn, acc);
		std::swap(m_hndl, l_hndl);
		close(l_hndl);
	}

	Service Service::Manager::create_service(const Service::Create_t & info) const
	{
		LogTrace();
		bool delayed = info.startType & 0x10000;
		SC_HANDLE hndl = CheckHandleErr(
			::CreateServiceW(m_hndl, info.name.c_str(),
				info.displayName,
				SERVICE_ALL_ACCESS,
				info.serviceType,
				info.startType & 0x0000000F,
				info.errorControl,
				info.binaryPathName.c_str(),
				info.loadOrderGroup,
				info.tagId,
				info.dependencies,
				nullptr,
				nullptr)
		);
		Service tmp(hndl);
		if (delayed)
			tmp.set_delayed(delayed);
		return std::move(tmp);
	}

	bool Service::Manager::is_exist(PCWSTR name) const
	{
		LogTrace();
		SC_HANDLE hndl = ::OpenServiceW(m_hndl, name, SERVICE_QUERY_STATUS);
		if (hndl)
		::CloseServiceHandle(hndl);
		return hndl;
	}

	SC_HANDLE Service::Manager::open(RemoteConnection * conn, ACCESS_MASK acc)
	{
		LogTrace();
		return CheckHandleErr(::OpenSCManagerW(conn->get_host().c_str(), nullptr, acc));
	}

	void Service::Manager::close(SC_HANDLE scm)
	{
		LogTrace();
		if (scm)
		::CloseServiceHandle(scm);
	}

}
