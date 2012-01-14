#include "farplugin.hpp"

ustring make_path(const ustring & path, const ustring & name) {
	return path + PATH_SEPARATOR + name;
}

void cut_to(PWSTR s, WCHAR symbol, bool bInclude = false) {
	for (ssize_t i = Len(s) - 1; i >= 0; --i)
		if (s[i] == symbol) {
			bInclude ? s[i] = L'\0' : s[i+1] = L'\0';
			break;
		}
}

void cut_to_slash(PWSTR s) {
	cut_to(s, L'\\', false);
}

Options::Options() {
}

void Options::load() {
	WCHAR iniPath[MAX_PATH_LEN];
	Copy(iniPath, Far::psi().ModuleName, lengthof(iniPath));
	cut_to_slash(iniPath);
	Cat(iniPath, L"rcl2apps.ini", lengthof(iniPath));
	Button = ::GetPrivateProfileIntW(L"Options", L"Button", 2, iniPath);
//	DWORD dwPanelSettings = Far::get_panel_settings();
//	DWORD dwInterfaceSettings = Far::get_interface_settings();
//	Top = 1 + (bool)(dwPanelSettings & FPS_SHOWCOLUMNTITLES) + (bool)(dwInterfaceSettings & FIS_ALWAYSSHOWMENUBAR);
//	Bottom = 1 + ((dwPanelSettings & FPS_SHOWSTATUSLINE) ? 2 : 0);
}
