#include <libext/service.hpp>
#include <libbase/pcstr.hpp>


using Base::Str::compare_cs;


namespace Ext {

	Service::Config_t::Config_t():
		serviceType(SERVICE_NO_CHANGE),
		startType(SERVICE_NO_CHANGE),
		errorControl(SERVICE_NO_CHANGE),
		binaryPathName(nullptr),
		loadOrderGroup(nullptr),
		tagId(nullptr),
		dependencies(nullptr),
		displayName(nullptr)
	{
	}

	void Service::Config_t::set_type(Type_t n, Type_t o) {
		if (n != o)
			serviceType = n;
	}

	void Service::Config_t::set_start(Start_t n, Start_t o) {
		if (n != o)
			startType = n;
	}

	void Service::Config_t::set_error_control(Error_t n, Error_t o) {
		if (n != o)
			errorControl = n;
	}

	void Service::Config_t::set_path(PCWSTR n, PCWSTR o) {
		if (compare_cs(n, o) != 0)
			binaryPathName = n;
	}

	void Service::Config_t::set_group(PCWSTR n, PCWSTR o) {
		if (compare_cs(n, o) != 0)
			loadOrderGroup = n;
	}

	void Service::Config_t::set_tag(DWORD & n, DWORD o) {
		if (n != o)
			tagId = &n;
	}

	void Service::Config_t::set_dependencies(PCWSTR n, PCWSTR o) {
		if (compare_cs(n, o) != 0)
			dependencies = n;
	}

	void Service::Config_t::set_display_name(PCWSTR n, PCWSTR o) {
		if (compare_cs(n, o) != 0)
			displayName = n;
	}

}
