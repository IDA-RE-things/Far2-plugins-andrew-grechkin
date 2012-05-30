#include "farplugin.hpp"
#include "lang.hpp"
#include "guid.hpp"

#include <API_far3/DlgBuilder.hpp>

windef::shared_ptr<FarPlugin> plugin;

bool FarPlugin::Execute() const {
	return true;
}

GUID FarPlugin::get_guid() {
	return PluginGuid;
}

PCWSTR FarPlugin::get_prefix() const {
	static PCWSTR ret = L"svcmgr";
	return ret;
}

PCWSTR FarPlugin::get_name() {
	return L"svcmgr";
}

PCWSTR FarPlugin::get_description() {
	return L"Windows services manager. FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	Far::helper_t::inst().init(FarPlugin::get_guid(), psi);
	options.load();
}

void FarPlugin::get_info(PluginInfo * pi) const {
//	Far::mbox(L"FarPlugin::GetPluginInfoW()");
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_NONE;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};

	if (options.AddToPluginsMenu) {
		pi->PluginMenu.Guids = PluginMenuGuids;
		pi->PluginMenu.Strings = PluginMenuStrings;
		pi->PluginMenu.Count = lengthof(PluginMenuStrings);
	}

	static PCWSTR DiskStrings[] = {Far::get_msg(Far::DiskTitle),};
	if (options.AddToDisksMenu) {
		pi->DiskMenu.Guids = PluginMenuGuids;
		pi->DiskMenu.Strings = DiskStrings;
		pi->DiskMenu.Count = lengthof(DiskStrings);
	}

	pi->PluginConfig.Guids = PluginMenuGuids;
	pi->PluginConfig.Strings = PluginMenuStrings;
	pi->PluginConfig.Count = lengthof(PluginMenuStrings);
	pi->CommandPrefix = options.Prefix;
}

HANDLE FarPlugin::open(const OpenInfo * Info)
{
	return ServicePanel::create(Info);
}

void FarPlugin::close(HANDLE hndl) {
	static_cast<Far::IPanel*>(hndl)->destroy();
}

int FarPlugin::configure() {
	PluginDialogBuilder Builder(Far::psi(), get_guid(), ConfigDialogGuid, Far::DlgTitle, nullptr);
	Builder.AddCheckbox(txtAddToPluginsMenu, &options.AddToPluginsMenu);
	Builder.AddCheckbox(txtAddToDiskMenu, &options.AddToDisksMenu);
	Builder.AddTextBefore(txtPluginPrefix,
		Builder.AddEditField(options.Prefix, lengthof(options.Prefix)));
	Builder.AddTextBefore(txtTimeout,
		Builder.AddFixEditField(options.Timeout, lengthof(options.Timeout), -1, L"99"));
	Builder.AddOKCancel(Far::txtBtnOk, Far::txtBtnCancel);

	if (Builder.ShowDialog()) {
		options.save();
		return true;
	}
	return false;
}
