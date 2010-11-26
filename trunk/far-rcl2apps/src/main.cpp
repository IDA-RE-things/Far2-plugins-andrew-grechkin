#include <win_std.h>

#include <far/helper.hpp>

typedef BOOL (WINAPI *READCONIN)(HANDLE, PINPUT_RECORD, DWORD, PDWORD);

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;

HANDLE		hInsuranceEvent;
DWORD		ControlKeyState;
LONG		Top, Bottom;
DWORD		Button;
BOOL		Flag;
READCONIN	Real_ReadConsoleInputW;

bool		IsPanel(COORD Pos) {
	WindowInfo	wInfo = {0};
	wInfo.Pos = -1;
	return	true;
//	WinMem::Alloc(wInfo.TypeName, MAX_PATH * sizeof(WCHAR));
//	WinMem::Alloc(wInfo.Name, MAX_PATH * sizeof(WCHAR));

	psi.AdvControl(psi.ModuleNumber, ACTL_GETWINDOWINFO, &wInfo);
	if (wInfo.Type == WTYPE_PANELS) {
		return	true;
		PanelInfo pnInfo;
		PanelInfo pnOtherInfo;

		psi.AdvControl((INT_PTR)INVALID_HANDLE_VALUE, ACTL_GETPANELSETTINGS, &pnInfo);
		psi.AdvControl((INT_PTR)INVALID_HANDLE_VALUE, ACTL_GETPANELSETTINGS, &pnOtherInfo);

		if ((Pos.X > pnInfo.PanelRect.left) &&
				(Pos.X < pnInfo.PanelRect.right) &&
				(Pos.Y >= pnInfo.PanelRect.top + Top) &&
				(Pos.Y <= pnInfo.PanelRect.bottom - Bottom) &&
				pnInfo.Visible) {
			if (!pnInfo.Plugin || ((pnInfo.Flags & PFLAGS_REALNAMES) == PFLAGS_REALNAMES))
				return	true;
		}

		if ((Pos.X > pnOtherInfo.PanelRect.left) &&
				(Pos.X < pnOtherInfo.PanelRect.right) &&
				(Pos.Y >= pnOtherInfo.PanelRect.top + Top) &&
				(Pos.Y <= pnOtherInfo.PanelRect.bottom - Bottom) &&
				pnOtherInfo.Visible) {
			if (!pnOtherInfo.Plugin || ((pnOtherInfo.Flags & PFLAGS_REALNAMES) == PFLAGS_REALNAMES))
				return	true;
		}
	}
	return	false;
}

void		CutTo(PWSTR s, WCHAR symbol, bool bInclude = false) {
	for (ssize_t i = Len(s) - 1; i >= 0; --i)
		if (s[i] == symbol) {
			bInclude ? s[i] = L'\0' : s[i+1] = L'\0';
			break;
		}

}

void		CutToSlash(PWSTR s) {
	CutTo(s, L'\\', false);
}

//extern void WINAPI ProcessInput(INPUT_RECORD *InRec);
PROC		RtlHookImportTable(PCSTR lpModuleName, PCSTR lpFunctionName, PROC pfnNew, HMODULE hModule) {
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

	return nullptr; //error
}

void WINAPI ProcessInput(INPUT_RECORD *InRec) {
	DWORD	dwReadCount;
	if ((InRec->EventType == MOUSE_EVENT) && !InRec->Event.MouseEvent.dwEventFlags) {
		if (InRec->Event.MouseEvent.dwButtonState == Button) {
			if (IsPanel(InRec->Event.MouseEvent.dwMousePosition)) {
				InRec->Event.MouseEvent.dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
				ControlKeyState = InRec->Event.MouseEvent.dwControlKeyState;
				Flag = 1;

				WriteConsoleInput(::GetStdHandle(STD_INPUT_HANDLE), InRec, 1, &dwReadCount);
			}
		}
		if (Flag && !InRec->Event.MouseEvent.dwButtonState) {
			BYTE OutRec[] = {
				0x01, 0x00, 0x52, 0xCA, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
				0x5D, 0x00, 0x5D, 0x00, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00
			};
			INPUT_RECORD	*rec = (INPUT_RECORD*)OutRec;
			rec->Event.KeyEvent.dwControlKeyState = ControlKeyState;
			::WriteConsoleInput(::GetStdHandle(STD_INPUT_HANDLE), rec, 1, &dwReadCount);
			Flag = 0;
		}
	}
}

BOOL WINAPI	Thunk_ReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, PDWORD lpNumberOfEventsRead) {
	BOOL bResult = Real_ReadConsoleInputW(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
	for (size_t i = 0; i < *lpNumberOfEventsRead; ++i)
		ProcessInput(&lpBuffer[i]);
	return	bResult;
}

///========================================================================================== Export
void WINAPI			EXP_NAME(ExitFAR)() {
	RtlHookImportTable("kernel32.dll", "ReadConsoleInputW", (PROC)Real_ReadConsoleInputW, GetModuleHandle(nullptr));
}

void WINAPI			EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_PRELOAD;
}

void WINAPI			EXP_NAME(SetStartupInfo)(const PluginStartupInfo *pInfo) {
	InitFSF(pInfo);
	DWORD dwPanelSettings;
	DWORD dwInterfaceSettings;
	WCHAR	IniName[MAX_PATH];
	Copy(IniName, pInfo->ModuleName, sizeofa(IniName));
	CutToSlash(IniName);
	Cat(IniName, L"rcl2apps.ini");
	Button = ::GetPrivateProfileIntW(L"Options", L"Button", 2, IniName);
	dwPanelSettings		= psi.AdvControl(psi.ModuleNumber, ACTL_GETPANELSETTINGS, 0);
	dwInterfaceSettings	= psi.AdvControl(psi.ModuleNumber, ACTL_GETINTERFACESETTINGS, 0);
	Top = 1 + (bool)(dwPanelSettings & FPS_SHOWCOLUMNTITLES) + (bool)(dwInterfaceSettings & FIS_ALWAYSSHOWMENUBAR);
	Bottom = 1 + ((dwPanelSettings & FPS_SHOWSTATUSLINE) ? 2 : 0);
	Real_ReadConsoleInputW = (READCONIN)RtlHookImportTable("kernel32.dll", "ReadConsoleInputW", (PROC)Thunk_ReadConsoleInputW, ::GetModuleHandle(nullptr));
}

extern "C" {
	BOOL WINAPI		DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved) {
		if (dwReason == DLL_PROCESS_ATTACH) {
			WCHAR	lpEventName[MAX_PATH];
			WCHAR	lpProcessId[MAX_PATH];
			Num2Str(lpProcessId, GetCurrentProcessId(), 16);
			Copy(lpEventName, L"__RCL2APPS__", sizeofa(lpEventName));
			Cat(lpEventName, lpProcessId, sizeofa(lpEventName));
			hInsuranceEvent = ::CreateEvent(nullptr, false, false,	lpEventName);
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				::SetEvent(hInsuranceEvent);
				return	false;
			}
		}

		if (dwReason == DLL_PROCESS_DETACH)
			::CloseHandle(hInsuranceEvent);

		return	true;
	}
}
