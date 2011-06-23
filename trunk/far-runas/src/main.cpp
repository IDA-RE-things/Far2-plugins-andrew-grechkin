/**
	runas: Run As FAR plugin
	Allow to run applications from other user

	© 2010 Andrew Grechkin

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

#include <libwin_def/win_def.h>

#include <far/helper.hpp>

#include <shlwapi.h>
#include <lm.h>

///========================================================================================== define
extern "C" {
	WINADVAPI BOOL WINAPI	IsTokenRestricted(HANDLE TokenHandle);
	WINADVAPI BOOL APIENTRY	CreateRestrictedToken(HANDLE ExistingTokenHandle, DWORD Flags,
			DWORD DisableSidCount, PSID_AND_ATTRIBUTES SidsToDisable,
			DWORD DeletePrivilegeCount, PLUID_AND_ATTRIBUTES PrivilegesToDelete,
			DWORD RestrictedSidCount, PSID_AND_ATTRIBUTES SidsToRestrict,
			PHANDLE NewTokenHandle);
};

#ifndef DISABLE_MAX_PRIVILEGE
#define DISABLE_MAX_PRIVILEGE   0x1
#endif

PCWSTR prefix = L"runas";

///========================================================================================== struct
enum		{
	MUsername = 5,
	MPasword,
	MRestricted,
	MCommandLine,
	MError,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;
//PluginOptions					Options;


bool			InitUsers(FarList &users) {
	DWORD dwLevel = 3;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;

	USER_INFO_3 *psi = nullptr;
	nStatus = ::NetUserEnum(nullptr, dwLevel,
							FILTER_NORMAL_ACCOUNT,
							(PBYTE*) & psi,
							MAX_PREFERRED_LENGTH,
							&dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
	if (nStatus == NERR_Success) {
		users.ItemsNumber = 0;
		WinMem::Alloc(users.Items, sizeof(*users.Items) * dwEntriesRead);
		for (DWORD i = 0; i < dwEntriesRead; ++i) {
			if (!WinFlag::Check(psi[i].usri3_flags, (DWORD)UF_ACCOUNTDISABLE)) {
				users.Items[users.ItemsNumber].Text = AssignStr(psi[i].usri3_name);
				if (psi[i].usri3_priv == USER_PRIV_ADMIN) {
					WinFlag::Set(users.Items[users.ItemsNumber].Flags, (DWORD)LIF_CHECKED);
				}
				++users.ItemsNumber;
			}
		}
		::NetApiBufferFree(psi);
	}
	return	nStatus == NERR_Success;
}
bool			FreeUsers(FarList &users) {
	for (int i = 0; i < users.ItemsNumber; ++i) {
		WinMem::Free(users.Items[i].Text);
	}
	WinMem::Free(users.Items);
	return	true;
}

PSID GetAdminSid() {
	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
	PSID ret = nullptr;

	::AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0, &ret);
	return ret;
}

HRESULT			ExecAsUser(PCWSTR app, PCWSTR user, PCWSTR pass) {
	AutoUTF	cmd(Expand(app));

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	if (::CreateProcessWithLogonW(user, nullptr, pass, LOGON_WITH_PROFILE, nullptr, (PWSTR)cmd.c_str(),
								  CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE,
								  nullptr, nullptr, &si, &pi)) {
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
		return	NO_ERROR;
	}
	return	::GetLastError();
}
HRESULT			ExecRestricted(PCWSTR app) {
	AutoUTF	cmd(Expand(app));

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	WinToken	hToken(TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT);
	if (hToken.IsOK()) {
		HANDLE	hTokenRest = nullptr;
		PSID	AdminSid = GetAdminSid();
		SID_AND_ATTRIBUTES	SidsToDisable[] = {
			{AdminSid, 0},
		};
		//TODO сделать restricted DACL
		if (::CreateRestrictedToken(hToken, DISABLE_MAX_PRIVILEGE, sizeofa(SidsToDisable), SidsToDisable, 0, nullptr, 0, nullptr, &hTokenRest)) {
			if (::CreateProcessAsUserW(hTokenRest, nullptr, (PWSTR)cmd.c_str(), nullptr, nullptr, false,
									   NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
				::CloseHandle(pi.hThread);
				::CloseHandle(pi.hProcess);
				::CloseHandle(hTokenRest);
				return	NO_ERROR;
			}
			::CloseHandle(hTokenRest);
		}
		::FreeSid(AdminSid);
	}
	return	::GetLastError();
}

///========================================================================================== Export
void	WINAPI	EXP_NAME(ClosePlugin)(HANDLE hPlugin) {
//	delete(PList*)hPlugin;
}
int		WINAPI	EXP_NAME(Configure)(int) {
//	Options.Write();
	return	true;
}
void	WINAPI	EXP_NAME(ExitFAR)() {
}
int		WINAPI	EXP_NAME(GetMinFarVersion)() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
void	WINAPI	EXP_NAME(GetPluginInfo)(PluginInfo *pi) {
	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = 0;

//	static const TCHAR	*DiskStrings[1];
//	static int			DiskNumbers[1] = {6};
//	DiskStrings[0] = GetMsg(DTitle);
//	pi->DiskMenuStrings = DiskStrings;
//	pi->DiskMenuNumbers = DiskNumbers;
//	pi->DiskMenuStringsNumber = sizeofa(DiskStrings);
	pi->DiskMenuStringsNumber = 0;

	static const TCHAR	*MenuStrings[1];
	MenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

//	pi->PluginConfigStrings = MenuStrings;
//	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = prefix;
}
HANDLE	WINAPI	EXP_NAME(OpenFilePlugin)(const TCHAR *Name, const unsigned char *Data, int DataSize, int OpMode) {
	return	INVALID_HANDLE_VALUE;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
//	Options.Read();

	AutoUTF	cline;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		FarPnl	pi(PANEL_ACTIVE);
		if (pi.IsOK()) {
			AutoUTF	buf(MAX_PATH_LEN, L'\0');
			fsf.GetCurrentDirectory(buf.capacity(), (PWSTR)buf.c_str());
			if (!buf.empty())
				::PathAddBackslash((PWSTR)buf.c_str());

			PluginPanelItem &PPI = pi[pi.CurrentItem()];
			buf += PPI.FindData.lpwszFileName;
			cline = buf;
		}
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		cline = (PCWSTR)Item;
	}
	FarList	users;
	if (InitUsers(users)) {
		enum {
			HEIGHT = 14,
			WIDTH = 48,
		};
		InitDialogItemF	Items[] = {
			{DI_DOUBLEBOX, 3, 1, WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)DlgTitle},
			{DI_TEXT,      5, 2, 0,  0,  0, (PCWSTR)MUsername},
			{DI_COMBOBOX,  5, 3, 42, 0,  DIF_SELECTONENTRY, L""},
			{DI_TEXT,      5, 4, 0,  0,  0, (PCWSTR)MPasword},
			{DI_PSWEDIT,   5, 5, 42, 0,  0, L""},
			{DI_CHECKBOX , 5, 6, 42, 0,  0, (PCWSTR)MRestricted},
			{DI_TEXT,      0, 7, 0,  0,  DIF_SEPARATOR, L""},
			{DI_TEXT,      5, 8, 0,  0,  0, (PCWSTR)MCommandLine},
			{DI_EDIT,      5, 9, 42, 0,  DIF_HISTORY, cline.c_str()},
			{DI_TEXT,      0,  HEIGHT - 4, 0,  0,  DIF_SEPARATOR,   L""},
			{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
			{DI_BUTTON,    0,  HEIGHT - 3, 0,  0,  DIF_CENTERGROUP, (PCWSTR)txtBtnCancel},
		};
		size_t	size = sizeofa(Items);
		FarDialogItem FarItems[size];
		InitDialogItemsF(Items, FarItems, size);
		FarItems[size - 2].DefaultButton = 1;
		FarItems[2].ListItems = &users;
		FarItems[8].History = L"runas.comline";

		FarDlg hDlg;
		if (hDlg.Init(psi.ModuleNumber, -1, -1, WIDTH, HEIGHT, L"Contents", FarItems, size)) {
			HRESULT	err = NO_ERROR;
			while (true) {
				int		ret = hDlg.Run();
				if (ret > 0 && Items[ret].Data == (PCWSTR)txtBtnOk) {
					AutoUTF	cmd(hDlg.Str(8));
					if (hDlg.Check(5)) {
						err = ExecRestricted(cmd.c_str());
					} else {
						AutoUTF	user(hDlg.Str(2));
						AutoUTF	pass(hDlg.Str(4));
						err = ExecAsUser(cmd.c_str(), user.c_str(), pass.c_str());
					}
					if (err == NO_ERROR) {
						break;
					} else {
						PCWSTR Msg[] = {GetMsg(MError), cmd.c_str(), L"", GetMsg(txtBtnOk), };
						::SetLastError(err);
						psi.Message(psi.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE,
									L"Contents", Msg, sizeofa(Msg), 1);
					}
				} else {
					break;
				}
			}
		}
		FreeUsers(users);
	}
	return	INVALID_HANDLE_VALUE;
}
void	WINAPI	EXP_NAME(SetStartupInfo)(const PluginStartupInfo *psi) {
	InitFSF(psi);
}

///========================================================================================= WinMain
extern		"C" {
	BOOL		WINAPI	DllMainCRTStartup(HANDLE, DWORD, PVOID) {
		return	true;
	}
}
