#include <libfar3/globalinfo_i.hpp>

#include <libbase/logger.hpp>


namespace Far {

	///================================================================================ GlobalInfo_i
	GlobalInfo_i::~GlobalInfo_i() {
		LogTrace();
	}

	void GlobalInfo_i::GetGlobalInfo(GlobalInfo * info) const {
		LogTrace();
		info->StructSize = sizeof(*info);
		info->MinFarVersion = FARMANAGERVERSION;
		info->Version = get_version();
		info->Guid = get_guid();
		info->Title = get_name();
		info->Description = get_description();
		info->Author = get_author();
	}

	int GlobalInfo_i::Configure(const ConfigureInfo * /*Info*/) {
		LogTrace();
		return 0;
	}

}
