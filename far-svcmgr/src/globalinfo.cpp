#include "guid.hpp"
#include "globalinfo.hpp"

#include "version.h"


FarGlobalInfo * FarGlobalInfo::create() {
	return new FarGlobalInfo;
}

void FarGlobalInfo::destroy() {
	delete this;
}


GUID FarGlobalInfo::get_guid() const {
	return PluginGuid;
}

PCWSTR FarGlobalInfo::get_name() const {
	return L"svcmgr";
}

PCWSTR FarGlobalInfo::get_description() const {
	return L"Windows services manager. FAR3 plugin";
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2012 Andrew Grechkin";
}

PCWSTR FarGlobalInfo::get_prefix() const {
	static PCWSTR ret = L"svcmgr";
	return ret;
}

void FarGlobalInfo::GetGlobalInfo(GlobalInfo * info) const {
	using namespace AutoVersion;
	info->StructSize = sizeof(*info);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(MAJOR, MINOR, BUILD, REVISION, VS_RELEASE);
	info->Guid = get_guid();
	info->Title = get_name();
	info->Description = get_description();
	info->Author = get_author();
}

FarGlobalInfo::~FarGlobalInfo() {
}

FarGlobalInfo::FarGlobalInfo() {
}


Base::shared_ptr<Far::GlobalInfo_i> globalInfo;
