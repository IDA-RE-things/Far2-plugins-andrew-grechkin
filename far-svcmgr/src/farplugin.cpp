#include "farplugin.hpp"

#ifndef FAR2
#include "guid.hpp"
#endif

winstd::shared_ptr<FarPlugin> plugin;

enum {
	txtBtnLogonAs = 5,
	txtBtnDepends,
	txtAddToPluginsMenu,
	txtAddToDiskMenu,
	txtDisksMenuHotkey,
	txtPluginPrefix,
	txtTimeout,

	txtSelectComputer,
	txtHost,
	txtEmptyForLocal,
	txtLogin,
	txtPass,
	txtEmptyForCurrent,
	txtConnecting,

	txtStopped,
	txtStartPending,
	txtStopPending,
	txtxRunning,
	txtContinuePending,
	txtPausePending,
	txtPaused,

	txtBoot,
	txtSystem,
	txtAuto,
	txtManual,
	txtDisbld,

	txtIgnore,
	txtNormal,
	txtSevere,
	txtCritical,

	txtAreYouSure,
	txtDeleteService,

	txtWaitAMoment,
	txtActionInProcess,
	txtStartingService,
	txtPausingService,
	txtStoppingService,
	txtRestartingService,
	txtContinueService,

	txtServices,
	txtDevices,

	txtDlgCreateService,
	txtDlgServiceProperties,
	txtDlgName,
	txtDlgDisplayName,
	txtDlgBinaryPath,
	txtDlgGroup,
	txtDlgServiceType,
	txtDlgStartupType,
	txtDlgErrorControl,

	txtDlgBoot,
	txtDlgSystem,
	txtDlgAuto,
	txtDlgManual,
	txtDlgDisbld,

	txtDlgIgnore,
	txtDlgNormal,
	txtDlgSevere,
	txtDlgCritical,

	txtDriver,
	txtFileSystemDriver,
	txtOwnProcess,
	txtSharedProcess,

	txtClmName,
	txtClmDisplayName,
	txtClmStatus,
	txtClmStart,
	txtClmLogon,
	txtClmDep,

	txtBtnStart,
	txtBtnConnct,
	txtBtnPause,
	txtBtnStop,
	txtBtnRestrt,
	txtBtnCreate,
	txtBtnStartP,
	txtBtnLocal,
	txtBtnContin,
	txtBtnDelete,
	txtBtnLogon,

	txtDlgLogonAs,
	txtDlgNetworkService,
	txtDlgLocalService,
	txtDlgLocalSystem,
	txtDlgAllowDesktop,
	txtDlgUserDefined,

	txtMnuTitle,
	txtMnuCommands,

	txtMnuSelectNewTitle,
	txtMnuSelectNewCommands,
};

ustring make_path(const ustring & path, const ustring & name) {
	return path + PATH_SEPARATOR + name;
}

bool FarPlugin::Execute() const {
	return true;
}

#ifndef FAR2
GUID FarPlugin::get_guid() {
	return PluginGuid;
}
#endif

PCWSTR FarPlugin::get_name() {
	return L"svcmgr";
}

PCWSTR FarPlugin::get_description() {
	return L"Manage services. FAR2, FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
#ifndef FAR2
	Far::helper_t::inst().init(PluginGuid, psi);
	options.load();
#else
	Far::helper_t::inst().init(psi);
	options.load(make_path(psi->RootKey, plugin->get_name()));
#endif
}

void FarPlugin::get_info(PluginInfo * pi) const {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = 0;

	static PCWSTR DiskStrings[] = {Far::get_msg(Far::DiskTitle),};
	if (options.AddToDisksMenu) {
		pi->DiskMenuStrings = DiskStrings;
		pi->DiskMenuStringsNumber = lengthof(DiskStrings);
	}

	static PCWSTR MenuStrings[] = {Far::get_msg(Far::MenuTitle),};;
	if (options.AddToPluginsMenu) {
#ifndef FAR2
		pi->PluginMenu.Guids = &MenuGuid;
		pi->PluginMenu.Strings = MenuStrings;
		pi->PluginMenu.Count = lengthof(MenuStrings);
#else
		pi->PluginMenuStrings = MenuStrings;
		pi->PluginMenuStringsNumber = lengthof(MenuStrings);
#endif
	}

	pi->PluginConfigStrings = MenuStrings;
	pi->PluginConfigStringsNumber = lengthof(MenuStrings);
	pi->CommandPrefix = options.Prefix.c_str(); //FIXME ptr can change
}

#ifndef FAR2
HANDLE FarPlugin::open(const OpenInfo * /*Info*/)
#else
    HANDLE FarPlugin::open(int /*OpenFrom*/, INT_PTR /*Item*/)
#endif
{
//	options.Read();
	return (HANDLE)ServicePanel::create_panel();
}

void FarPlugin::close(HANDLE hndl) {
	delete static_cast<IFarPanel*>(hndl);
}

int FarPlugin::configure() {
//	static WCHAR	DiskDigit[2] = {0};
//	static WCHAR	Timeout[3] = {0};
//	options.Read();
//	Num2Str(Timeout, Options.TimeOut / 1000);
//
//	enum {
//		HEIGHT = 11,
//		WIDTH = 56,
//	};
//	InitDialogItemF Items[] = {
//		{DI_DOUBLEBOX, 3,  1,  WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
//		{DI_CHECKBOX,  5, 2, 0,  0, 0, (PCWSTR)txtAddToPluginsMenu},
//		{DI_CHECKBOX,  5, 3, 0,  0, 0, (PCWSTR)txtAddToDiskMenu},
//		{DI_FIXEDIT,   7, 4, 7,  4, DIF_MASKEDIT, DiskDigit},
//		{DI_TEXT,     10, 4, 0,  0, 0, (PCWSTR)txtDisksMenuHotkey},
//		{DI_EDIT,      5, 5, 12, 5, 0, Options.Prefix.c_str()},
//		{DI_TEXT,     14, 5, 0,  0, 0, (PCWSTR)txtPluginPrefix},
//		{DI_FIXEDIT,   5, 6, 6,  4, DIF_MASKEDIT, Timeout},
//		{DI_TEXT,     14, 6, 0,  0, 0, (PCWSTR)txtTimeout},
//		{DI_TEXT,      5, 7, 0,  0, DIF_SEPARATOR,   L""},
//		{DI_BUTTON,    0, 8, 0,  0, DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
//		{DI_BUTTON,    0, 8, 0,  0, DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
//	};
//	size_t	size = sizeofa(Items);
//
//	FarDialogItem FarItems[size];
//	InitDialogItemsF(Items, FarItems, size);
//	FarItems[1].Selected = Options.AddToPluginsMenu;
//	FarItems[2].Selected = Options.AddToDisksMenu;
//	FarItems[3].Mask = L"9";
//	FarItems[7].Mask = L"99";
//	FarItems[10].DefaultButton = 1;
//
//	Far::Dialog hDlg;
//	if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"dlgConfigure", FarItems, size)) {
//		int	ret = hDlg.Run();
//		if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
//			options.AddToPluginsMenu = GetCheck(hDlg, 1);
//			options.AddToDisksMenu = GetCheck(hDlg, 2);
//			options.Prefix = GetDataPtr(hDlg, 5);
//			options.TimeOut = AsInt(GetDataPtr(hDlg, 7)) * 1000;
//			options.save();
//		}
//	}
	return	true;
}
