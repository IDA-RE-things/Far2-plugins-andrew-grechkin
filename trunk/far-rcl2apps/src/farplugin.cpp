#include "farplugin.hpp"
#include "lang.hpp"
#include "guid.hpp"

winstd::shared_ptr<FarPlugin> plugin;

FReadConsoleInputW Real_ReadConsoleInputW;

PROC RtlHookImportTable(PCSTR lpModuleName, PCSTR lpFunctionName, PROC pfnNew, HMODULE hModule) {
	PBYTE pModule = (PBYTE)hModule;
	PROC pfnResult = nullptr;

	//dword dwBase = (dword)hModule;
	//dword dwOP;

	PIMAGE_DOS_HEADER pDosHeader;
	pDosHeader = (PIMAGE_DOS_HEADER)pModule;

	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	PIMAGE_NT_HEADERS pPEHeader  = (PIMAGE_NT_HEADERS) & pModule[pDosHeader->e_lfanew];

	if (pPEHeader->Signature != 0x00004550)
		return nullptr;

	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) & pModule[pPEHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress];

	PCSTR lpImportTableFunctionName;

	if (!pPEHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
		return nullptr;

	PCSTR lpImportTableModuleName;

	while (pImportDesc->Name) {
		lpImportTableModuleName = (PCSTR) & pModule[pImportDesc->Name];
		if (!Cmpi(lpImportTableModuleName, lpModuleName))
			break;
		pImportDesc++;
	}

	if (!pImportDesc->Name)
		return nullptr;

	PIMAGE_THUNK_DATA pFirstThunk;
	PIMAGE_THUNK_DATA pOriginalThunk;

	pFirstThunk = (PIMAGE_THUNK_DATA) & pModule[pImportDesc->FirstThunk];
	pOriginalThunk = (PIMAGE_THUNK_DATA) & pModule[pImportDesc->OriginalFirstThunk];

	while (pFirstThunk->u1.Function) {
		size_t index = (size_t)((PIMAGE_IMPORT_BY_NAME)pOriginalThunk->u1.AddressOfData)->Name;
		lpImportTableFunctionName = (PCSTR) & pModule[index];

		DWORD dwOldProtect;
		PROC* ppfnOld;
		if (!Cmpi(lpImportTableFunctionName, lpFunctionName)) {
			pfnResult = (PROC)pFirstThunk->u1.Function;
			ppfnOld = (PROC*) & pFirstThunk->u1.Function;
			VirtualProtect(ppfnOld, sizeof(PROC), PAGE_READWRITE, &dwOldProtect);
			WriteProcessMemory(GetCurrentProcess(), ppfnOld, &pfnNew, sizeof pfnNew, nullptr);
			VirtualProtect(ppfnOld, sizeof(PROC), dwOldProtect, &dwOldProtect);
			return pfnResult;
		}
		pFirstThunk++;
		pOriginalThunk++;
	}

	return nullptr;
}

BOOL WINAPI Thunk_ReadConsoleInputW(HANDLE console, PINPUT_RECORD buffer, DWORD length, PDWORD read) {
	BOOL ret = Real_ReadConsoleInputW(console, buffer, length, read);
	for (size_t i = 0; i < *read; ++i)
		plugin->process_input(&buffer[i]);
	return ret;
}

#ifndef FAR2
GUID FarPlugin::get_guid() {
	return PluginGuid;
}
#endif

PCWSTR FarPlugin::get_prefix() const {
	static PCWSTR ret = L"rcl2apps";
	return ret;
}

PCWSTR FarPlugin::get_name() {
	return L"rcl2apps";
}

PCWSTR FarPlugin::get_description() {
	return L"Maps mouse button to Apps key. FAR2, FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
	Real_ReadConsoleInputW = (FReadConsoleInputW)RtlHookImportTable("kernel32.dll", "ReadConsoleInputW", (PROC)Thunk_ReadConsoleInputW, ::GetModuleHandleW(nullptr));
#ifndef FAR2
	Far::helper_t::inst().init(PluginGuid, psi);
#else
	Far::helper_t::inst().init(psi);
#endif
	options.load();
}

void FarPlugin::get_info(PluginInfo * pi) const {
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_PRELOAD;
}

void FarPlugin::process_input(PINPUT_RECORD record) {
	if ((record->EventType == MOUSE_EVENT) && record->Event.MouseEvent.dwEventFlags == 0) {
		DWORD written;
		if (record->Event.MouseEvent.dwButtonState == options.Button) {
			if (is_panel(record->Event.MouseEvent.dwMousePosition)) {
				record->Event.MouseEvent.dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
				options.ControlKeyState = record->Event.MouseEvent.dwControlKeyState;
				options.Flag = 1;
				::WriteConsoleInputW(::GetStdHandle(STD_INPUT_HANDLE), record, 1, &written);
			}
		}
		if (options.Flag && record->Event.MouseEvent.dwButtonState == 0) {
			options.Flag = 0;
			INPUT_RECORD apps = {KEY_EVENT};
			apps.Event.KeyEvent.bKeyDown = TRUE;
			apps.Event.KeyEvent.wRepeatCount = 1;
			apps.Event.KeyEvent.wVirtualKeyCode = VK_APPS;
			apps.Event.KeyEvent.dwControlKeyState = options.ControlKeyState;
			::WriteConsoleInputW(::GetStdHandle(STD_INPUT_HANDLE), &apps, 1, &written);
			apps.Event.KeyEvent.bKeyDown = FALSE;
			::WriteConsoleInputW(::GetStdHandle(STD_INPUT_HANDLE), &apps, 1, &written);
		}
	}
}

bool FarPlugin::is_panel(COORD /*Pos*/) const {
	return	true;
//	WindowInfo	wInfo = {0};
//	wInfo.Pos = -1;
////	WinMem::Alloc(wInfo.TypeName, MAX_PATH * sizeof(WCHAR));
////	WinMem::Alloc(wInfo.Name, MAX_PATH * sizeof(WCHAR));
//
//	psi.AdvControl(psi.ModuleNumber, ACTL_GETWINDOWINFO, &wInfo);
//	if (wInfo.Type == WTYPE_PANELS) {
//		return	true;
//		PanelInfo pnInfo;
//		PanelInfo pnOtherInfo;
//
//		psi.AdvControl((INT_PTR)INVALID_HANDLE_VALUE, ACTL_GETPANELSETTINGS, &pnInfo);
//		psi.AdvControl((INT_PTR)INVALID_HANDLE_VALUE, ACTL_GETPANELSETTINGS, &pnOtherInfo);
//
//		if ((Pos.X > pnInfo.PanelRect.left) &&
//				(Pos.X < pnInfo.PanelRect.right) &&
//				(Pos.Y >= pnInfo.PanelRect.top + Top) &&
//				(Pos.Y <= pnInfo.PanelRect.bottom - Bottom) &&
//				pnInfo.Visible) {
//			if (!pnInfo.Plugin || ((pnInfo.Flags & PFLAGS_REALNAMES) == PFLAGS_REALNAMES))
//				return	true;
//		}
//
//		if ((Pos.X > pnOtherInfo.PanelRect.left) &&
//				(Pos.X < pnOtherInfo.PanelRect.right) &&
//				(Pos.Y >= pnOtherInfo.PanelRect.top + Top) &&
//				(Pos.Y <= pnOtherInfo.PanelRect.bottom - Bottom) &&
//				pnOtherInfo.Visible) {
//			if (!pnOtherInfo.Plugin || ((pnOtherInfo.Flags & PFLAGS_REALNAMES) == PFLAGS_REALNAMES))
//				return	true;
//		}
//	}
//	return	false;
}
