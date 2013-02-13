#include <libext/service.hpp>
#include <libext/exception.hpp>

#include <libbase/logger.hpp>


namespace Ext {

	///============================================================================= Service::Info_t
	Service::Info_t::Info_t(SC_HANDLE scm, const ENUM_SERVICE_STATUS_PROCESSW & st):
		name(st.lpServiceName),
		displayName(st.lpDisplayName),
		startType(Start_t::DISABLED),
		errorControl(Error_t::IGNORE_ERROR),
		tagId(0),
		status(st.ServiceStatusProcess)
	{
		try {
			Service svc(scm, name.c_str(), SERVICE_QUERY_CONFIG);
			Base::auto_buf<LPQUERY_SERVICE_CONFIGW> conf(svc.QueryConfig());
			binaryPathName = conf->lpBinaryPathName;
			loadOrderGroup = conf->lpLoadOrderGroup;
			dependencies = conf->lpDependencies;
			serviceStartName = conf->lpServiceStartName;
			startType = (Start_t)(conf->dwStartType | (svc.get_delayed() ? 0x10000 : 0));
			errorControl = (Error_t)conf->dwErrorControl;
			tagId = conf->dwTagId;
			description = svc.get_description();
		} catch (Ext::AbstractError & e) {
			LogWarn(L"exception cought: %s, %s\n", e.what().c_str(), e.where());
			LogWarn(L"service: %s unavalible\n", name.c_str());
			// skip query info
		}
	}

	Service::Info_t::Info_t(PCWSTR _name, const Service & svc):
		name(_name),
		startType(Start_t::DISABLED),
		errorControl(Error_t::IGNORE_ERROR),
		tagId(0),
		status({0})
	{
		LogTrace();
		try {
			Base::auto_buf<LPQUERY_SERVICE_CONFIGW> conf(svc.QueryConfig());
			displayName = conf->lpDisplayName;
			status = svc.get_status();
			binaryPathName = conf->lpBinaryPathName;
			loadOrderGroup = conf->lpLoadOrderGroup;
			dependencies = conf->lpDependencies;
			serviceStartName = conf->lpServiceStartName;
			startType = (Start_t)(conf->dwStartType | (svc.get_delayed() ? 0x10000 : 0));
			errorControl = (Error_t)conf->dwErrorControl;
			tagId = conf->dwTagId;
			description = svc.get_description();
		} catch (Ext::AbstractError & e) {
			LogWarn(L"exception cought: %s, %s\n", e.what().c_str(), e.where());
			LogWarn(L"service: %s unavalible handle: %p\n", name.c_str(), (SC_HANDLE)svc);
			// skip query info
		}
	}

	bool Service::Info_t::operator < (const Service::Info_t & rhs) const {
		return name < rhs.name;
	}

	bool Service::Info_t::operator == (const ustring & nm) const {
		return this->name == nm;
	}

}

