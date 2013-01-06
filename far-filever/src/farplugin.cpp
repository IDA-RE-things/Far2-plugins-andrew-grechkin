/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
	FAR3 plugin

	© 2013 Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <farplugin.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/panel.hpp>
#include <libfar3/obsolete.hpp>
#include <libbase/logger.hpp>
#include <libbase/memory.hpp>
#include <libbase/pcstr.hpp>

#include <globalinfo.hpp>
#include <guid.hpp>
#include <lang.hpp>
#include <fileversion.hpp>


Base::NamedValues<WORD> Machines[] = {
	{ L"UNKNOWN", IMAGE_FILE_MACHINE_UNKNOWN },
	{ L"I386", IMAGE_FILE_MACHINE_I386 },
	{ L"R4000", IMAGE_FILE_MACHINE_R4000 },
	{ L"WCEMIPSV2", IMAGE_FILE_MACHINE_WCEMIPSV2 },
	{ L"SH3", IMAGE_FILE_MACHINE_SH3 },
	{ L"SH3DSP", IMAGE_FILE_MACHINE_SH3DSP },
	{ L"SH4", IMAGE_FILE_MACHINE_SH4 },
	{ L"SH5", IMAGE_FILE_MACHINE_SH5 },
	{ L"ARM", IMAGE_FILE_MACHINE_ARM },
	{ L"THUMB", IMAGE_FILE_MACHINE_THUMB },
	{ L"AM33", IMAGE_FILE_MACHINE_AM33 },
	{ L"POWERPC", IMAGE_FILE_MACHINE_POWERPC },
	{ L"POWERPCFP", IMAGE_FILE_MACHINE_POWERPCFP },
	{ L"IA64", IMAGE_FILE_MACHINE_IA64 },
	{ L"MIPS16", IMAGE_FILE_MACHINE_MIPS16 },
	{ L"MIPSFPU", IMAGE_FILE_MACHINE_MIPSFPU },
	{ L"MIPSFPU16", IMAGE_FILE_MACHINE_MIPSFPU16 },
	{ L"EBC", IMAGE_FILE_MACHINE_EBC },
	{ L"AMD64", IMAGE_FILE_MACHINE_AMD64 },
	{ L"M32R", IMAGE_FILE_MACHINE_M32R },
};

///======================================================================================= FarPlugin
struct FarPlugin: public Far::Plugin_i {
	FarPlugin(const PluginStartupInfo * Info);

	~FarPlugin() override;

	void GetPluginInfo(PluginInfo * Info) override;

	Far::PanelController_i * Open(const OpenInfo * Info) override;
};


FarPlugin::FarPlugin(const PluginStartupInfo * Info):
	Far::Plugin_i(Info)
{
	LogTrace();
}

FarPlugin::~FarPlugin() {
	LogTrace();
}

void FarPlugin::GetPluginInfo(PluginInfo * Info) {
	LogTrace();
	Info->Flags = PF_NONE;

	static GUID PluginMenuGuids[] = {MenuGuid,};
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle),};

	Info->PluginMenu.Guids = PluginMenuGuids;
	Info->PluginMenu.Strings = PluginMenuStrings;
	Info->PluginMenu.Count = Base::lengthof(PluginMenuStrings);

	Info->CommandPrefix = get_global_info()->prefix;
}

Far::PanelController_i * FarPlugin::Open(const OpenInfo * Info)
{
	if (!version_dll::inst().is_valid()) {
		Far::ebox(L"Can't load version.dll");
		return (Far::PanelController_i * )INVALID_HANDLE_VALUE;
	}

	WCHAR buf[Base::MAX_PATH_LEN] = {0};
	if (Info->OpenFrom == OPEN_PLUGINSMENU || Info->OpenFrom == OPEN_FROMMACRO) {
		Far::Panel pi(PANEL_ACTIVE);
		if (pi.is_ok()) {
			const PluginPanelItem * ppi = pi.get_current();
			PCWSTR fileName = ppi->FileName;
			if (Base::Str::find(fileName, Base::PATH_SEPARATOR)) {
				Base::Str::copy(buf, fileName, Base::lengthof(buf));
			} else {
				Base::Str::copy(buf, pi.get_current_directory(), Base::lengthof(buf));
				if (!Base::Str::is_empty(buf)) {
					Far::fsf().AddEndSlash(buf);
				}
				Base::Str::cat(buf, fileName, Base::lengthof(buf));
			}
		}
	} else if (Info->OpenFrom == OPEN_COMMANDLINE) {
		Base::Str::copy(buf, (PCWSTR)Info->Data, Base::lengthof(buf));
	}
	Far::fsf().Trim(buf);
	Far::fsf().Unquote(buf);

	FileVersion fv(buf);
	if (fv.is_ok()) {
		FVI fvi(fv);
			int i = 0, x = 70, y = 2;
			Far::InitDialogItemF Items[] = {
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtFileFullVer},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fv.ver()},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtFileLang},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fv.lng()},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtMachine},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, Base::NamedValues<WORD>::GetName(Machines, lengthof(Machines), fv.machine())},
				{DI_TEXT, 5, y++, 0, 0,         DIF_SEPARATOR, L""},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)fvi[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fvi[i++].data},
				{DI_TEXT, 5, y++, 0, 0,         DIF_SEPARATOR, L""},
				{DI_BUTTON, 0, y++, 0, 0,       DIF_CENTERGROUP, (PCWSTR)Far::txtBtnOk},
				{DI_DOUBLEBOX, 3, 1, x, y,      0,               (PCWSTR)Far::DlgTitle},
			};

			size_t size = Base::lengthof(Items);
			FarDialogItem FarItems[size];
			InitDialogItemsF(Items, FarItems, size);
			HANDLE hndl = Far::psi().DialogInit(Far::get_plugin_guid(), &DialogGuid, -1, -1, x + 4, y + 2, L"Contents", FarItems, size, 0, 0, nullptr, 0);
			Far::psi().DialogRun(hndl);
			Far::psi().DialogFree(hndl);
	}
	return nullptr;
}

///=================================================================================================
Far::Plugin_i * create_FarPlugin(const PluginStartupInfo * psi) {
	return new FarPlugin(psi);
}

void destroy(Far::Plugin_i * plugin) {
	delete plugin;
}
