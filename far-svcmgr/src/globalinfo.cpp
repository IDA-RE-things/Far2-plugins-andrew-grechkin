#include "globalinfo.hpp"
#include "guid.hpp"
#include "lang.hpp"

#include <libfar3/DlgBuilder.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include "version.h"


Base::shared_ptr<FarGlobalInfo> globalInfo;


FarGlobalInfo::FarGlobalInfo():
	m_settings(nullptr)
{
	LogTrace();
	AddToPluginsMenu = 1;
	AddToDisksMenu = 0;
	TimeOut = 30;
	Base::copy_str(Prefix, L"svcmgr");
}

FarGlobalInfo::~FarGlobalInfo() {
	delete m_settings;
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

void FarGlobalInfo::GetGlobalInfo(GlobalInfo * info) const {
	LogTrace();
	using namespace AutoVersion;
	info->StructSize = sizeof(*info);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(MAJOR, MINOR, BUILD, REVISION, VS_RELEASE);
	info->Guid = get_guid();
	info->Title = get_name();
	info->Description = get_description();
	info->Author = get_author();
}


FarGlobalInfo * create_GlobalInfo() {
	return new FarGlobalInfo;
}

void destroy(FarGlobalInfo * info) {
	delete info;
}



void FarGlobalInfo::load_settings() {
	if (!m_settings)
		m_settings = new Far::Settings_t(get_guid());
	AddToPluginsMenu = m_settings->get(L"AddToPluginsMenu", AddToPluginsMenu);
	AddToDisksMenu = m_settings->get(L"AddToDisksMenu", AddToDisksMenu);
	TimeOut = m_settings->get(L"wait_timeout", TimeOut);
	Base::copy_str(Prefix, m_settings->get(L"Prefix", L"svcmgr"));
}

void FarGlobalInfo::save_settings() const {
	m_settings->set(L"AddToPluginsMenu", AddToPluginsMenu);
	m_settings->set(L"AddToDisksMenu", AddToDisksMenu);
	m_settings->set(L"wait_timeout", TimeOut);
	m_settings->set(L"Prefix", Prefix);
}

int FarGlobalInfo::Configure(const ConfigureInfo * /*Info*/) {
	Far::DialogBuilder builder = Far::get_dialog_builder(ConfigDialogGuid, Far::get_msg(Far::DlgTitle), nullptr);
	builder->add_checkbox(Far::get_msg(txtAddToPluginsMenu), &AddToPluginsMenu);
	builder->add_checkbox(Far::get_msg(txtAddToDiskMenu), &AddToDisksMenu);
	builder->add_text_before(Far::get_msg(txtPluginPrefix),
		builder->add_editfield(Prefix, Base::lengthof(Prefix)));
	builder->add_text_before(Far::get_msg(txtTimeout),
		builder->add_inteditfield(&TimeOut, 2));
	builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	if (builder->show()) {
		save_settings();
		return true;
	}
	return false;
}
