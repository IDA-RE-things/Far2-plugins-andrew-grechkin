/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog

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

///========================================================================================== define
PCWSTR prefix = L"fver";

///========================================================================================== struct
enum	FarMessage {
	MtxtFileFullVer = 5,
	MtxtFileLang,

	MtxtFileComment,
	MtxtFileCompany,
	MtxtFileDesc,
	MtxtFileVer,
	MtxtFileInternal,
	MtxtFileCopyright,
	MtxtFileTrade,
	MtxtFileOriginal,
	MtxtFilePrivate,
	MtxtFileProductName,
	MtxtFileProductVer,
	MtxtFileSpecial,
	MtxtMachine,
};

///======================================================================================= implement
PluginStartupInfo		psi;
FarStandardFunctions 	fsf;
//PluginOptions					Options;

NamedValues<WORD>	Machines[] = {
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

class		FileVersion {
	WCHAR	m_ver[32];
	WCHAR	m_lng[32];
	WCHAR	m_lngId[16];
	WCHAR	m_lngIderr[16];
	PBYTE	m_data;
	WORD	m_MajorVersion, m_MinorVersion;
	WORD	m_BuildNumber, m_RevisionNumber;

	WORD	m_machine;
	WORD	m_flags;
public:
	~FileVersion() {
		WinMem::Free(m_data);
	}
	FileVersion(PCWSTR path): m_data(nullptr) {
		WinMem::Zero(*this);

		DWORD	dwHandle, dwLen = ::GetFileVersionInfoSize(path, &dwHandle);
		if (dwLen) {
			if (WinMem::Alloc(m_data, dwLen)) {
				UINT	bufLen;
				VS_FIXEDFILEINFO	*psi;
				if (::GetFileVersionInfo(path, dwHandle, dwLen, m_data)) {
					if (::VerQueryValue(m_data, (PWSTR)L"\\", (PVOID*)&psi, &bufLen)) {
						m_MajorVersion	= HIWORD(psi->dwFileVersionMS);
						m_MinorVersion	= LOWORD(psi->dwFileVersionMS);
						m_BuildNumber		= HIWORD(psi->dwFileVersionLS);
						m_RevisionNumber	= LOWORD(psi->dwFileVersionLS);
						_snwprintf(m_ver, sizeofa(m_ver), L"%d.%d.%d.%d", m_MajorVersion, m_MinorVersion, m_BuildNumber, m_RevisionNumber);
					}
					struct LANGANDCODEPAGE {
						WORD wLanguage;
						WORD wCodePage;
					} *pTranslate;
					if (::VerQueryValue(m_data, (PWSTR)L"\\VarFileInfo\\Translation", (PVOID*)&pTranslate, &bufLen)) {
						::VerLanguageName(pTranslate->wLanguage, m_lng, sizeofa(m_lng));
						_snwprintf(m_lngId, sizeofa(m_lngId), L"%04x%04x", pTranslate->wLanguage, pTranslate->wCodePage);
						WCHAR	tmp[4] = {0};
						DWORD	errnum = 0;
						_snwprintf(tmp, sizeofa(tmp), L"%04x", pTranslate->wCodePage);
						errnum = AsUInt(tmp);
						_snwprintf(m_lngIderr, sizeofa(m_lngIderr), L"%04x%04x", pTranslate->wLanguage, errnum);
					}
				}
			}
		} else {
			return;
		}

		FileMap	fmap(path);
		if (!fmap.IsOK() || (fmap.size() < sizeof(IMAGE_DOS_HEADER))) {
			return;
		}
		fmap.Next();
		PIMAGE_DOS_HEADER	dosHeader = (PIMAGE_DOS_HEADER)fmap.data();
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
			return;
		}
		PIMAGE_NT_HEADERS	pPEHeader = (PIMAGE_NT_HEADERS)NTSIGNATURE(dosHeader);

		if (pPEHeader->Signature != IMAGE_NT_SIGNATURE) {
			return;
		}
		m_machine = pPEHeader->FileHeader.Machine;
		m_flags = pPEHeader->FileHeader.Characteristics;
	}
	PCWSTR			ver() const {
		return	m_ver;
	}
	PCWSTR			lng() const {
		return	m_lng;
	}
	PCWSTR			lngID() const {
		return	m_lngId;
	}
	PCWSTR			lngIDerr() const {
		return	m_lngIderr;
	}
	bool			IsOK() const {
		return	m_data;
	}
	const PVOID		GetData() const {
		return	m_data;
	}
	WORD			machine() const {
		return	m_machine;
	}
	bool			Is64Bit() const {
		return	m_machine == IMAGE_FILE_MACHINE_IA64 || m_machine == IMAGE_FILE_MACHINE_AMD64;
	};
};

struct		FileVerInfo_ {
	PWSTR		data;
	PCWSTR		SubBlock;
	FarMessage	msgTxt;
} FileVerInfo[] = {
	{nullptr, L"FileDescription", MtxtFileDesc},
	{nullptr, L"LegalCopyright", MtxtFileCopyright},
	{nullptr, L"Comments", MtxtFileComment},
	{nullptr, L"CompanyName", MtxtFileCompany},
	{nullptr, L"FileVersion", MtxtFileVer},
	{nullptr, L"InternalName", MtxtFileInternal},
	{nullptr, L"LegalTrademarks", MtxtFileTrade},
	{nullptr, L"OriginalFilename", MtxtFileOriginal},
	{nullptr, L"PrivateBuild", MtxtFilePrivate},
	{nullptr, L"ProductName", MtxtFileProductName},
	{nullptr, L"ProductVersion", MtxtFileProductVer},
	{nullptr, L"SpecialBuild", MtxtFileSpecial},
};

bool			InitDataArray(const FileVersion &in) {
	if (in.IsOK()) {
		WCHAR	QueryString[128] = {0};
		UINT	bufLen;
		for (size_t i = 0; i < sizeofa(FileVerInfo); ++i) {
			_snwprintf(QueryString, sizeofa(QueryString), L"\\StringFileInfo\\%s\\%s", in.lngID(), FileVerInfo[i].SubBlock);
			if (!::VerQueryValueW(in.GetData(), QueryString, (PVOID*)&(FileVerInfo[i].data), &bufLen)) {
				_snwprintf(QueryString, sizeofa(QueryString), L"\\StringFileInfo\\%s\\%s", in.lngIDerr(), FileVerInfo[i].SubBlock);
				if (!::VerQueryValueW(in.GetData(), QueryString, (PVOID*)&(FileVerInfo[i].data), &bufLen))
					FileVerInfo[i].data = (PWSTR)L"";
			}
		}
	}
	return	false;
}

///========================================================================================== export
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

//	static PCWSTR	DiskStrings[1];
//	static int		DiskNumbers[1] = {6};
//	DiskStrings[0] = GetMsg(DTitle);
//	pi->DiskMenuStrings = DiskStrings;
//	pi->DiskMenuNumbers = DiskNumbers;
//	pi->DiskMenuStringsNumber = sizeofa(DiskStrings);
	pi->DiskMenuStringsNumber = 0;

	static PCWSTR MenuStrings[1];
	MenuStrings[0] = GetMsg(MenuTitle);
	pi->PluginMenuStrings = MenuStrings;
	pi->PluginMenuStringsNumber = sizeofa(MenuStrings);

//	pi->PluginConfigStrings = MenuStrings;
//	pi->PluginConfigStringsNumber = sizeofa(MenuStrings);
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = prefix;
}
HANDLE	WINAPI	EXP_NAME(OpenFilePlugin)(const WCHAR *Name, const unsigned char *Data, int DataSize, int OpMode) {
	return	INVALID_HANDLE_VALUE;
}
HANDLE	WINAPI	EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item) {
	WCHAR	buf[MAX_PATH_LEN];
	if (OpenFrom == OPEN_PLUGINSMENU) {
		FarPnl	pi(PANEL_ACTIVE, FCTL_GETPANELINFO);
		if (pi.IsOK()) {
			PluginPanelItem ppi = pi[pi.CurrentItem()];
			if (Find(ppi.FindData.lpwszFileName, PATH_SEPARATOR)) {
				Copy(buf, ppi.FindData.lpwszFileName, sizeofa(buf));
			} else {
				Copy(buf, pi.CurDir(), sizeofa(buf));
				if (!Empty(buf))
					fsf.AddEndSlash(buf);
				Cat(buf, ppi.FindData.lpwszFileName, sizeofa(buf));
			}
		}

		/*
				PanelInfo	pi;
				if (psi.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, sizeof(pi), (LONG_PTR)&pi)) {
					CStrW	buf(MAX_PATH_LENGTH + MAX_PATH + 1);
					fsf.GetCurrentDirectory(buf.capacity(), buf.buffer());
					if (!buf.empty())
						fsf.AddEndSlash(buf.buffer());

					WinBuf<PluginPanelItem>	PPI(psi.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, nullptr), true);
					psi.Control(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)PPI.data());
					if (WinFlag<DWORD>::Check(pi.Flags, PFLAGS_REALNAMES)) {
						if (Find(PPI->FindData.lpwszFileName, PATH_SEPARATOR)) {
							buf = PPI->FindData.lpwszFileName;
						} else {
							buf += PPI->FindData.lpwszFileName;
						}
					}
					cline = buf;
				}
		*/
	} else if (OpenFrom == OPEN_COMMANDLINE) {
		Copy(buf, (PCWSTR)Item, sizeofa(buf));
	}
	fsf.Trim(buf);
	fsf.Unquote(buf);
	FileVersion fv(buf);
	if (fv.IsOK()) {
		InitDataArray(fv);
		if (true) {
			size_t i = 0, x = 70, y = 2;
			InitDialogItemF Items[] = {
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtFileFullVer},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fv.ver()},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtFileLang},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, fv.lng()},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)MtxtMachine},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, NamedValues<WORD>::GetName(Machines, sizeofa(Machines), fv.machine())},
				{DI_TEXT, 5, y++, 0, 0,         DIF_SEPARATOR, L""},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y, 26, 0,          0,            (PCWSTR)FileVerInfo[i].msgTxt},
				{DI_EDIT, 28, y++, x - 2, 0,    DIF_READONLY, FileVerInfo[i++].data},
				{DI_TEXT, 5, y++, 0, 0,         DIF_SEPARATOR, L""},
				{DI_BUTTON, 0, y++, 0, 0,       DIF_CENTERGROUP, (PCWSTR)txtBtnOk},
				{DI_DOUBLEBOX, 3, 1, x, y,      0,               (PCWSTR)DlgTitle},
			};

			size_t	size = sizeofa(Items);
			FarDialogItem FarItems[size];
			InitDialogItemsF(Items, FarItems, size);
			HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, x + 4, y + 2, L"Contents",
										 FarItems, size, 0, 0, nullptr, 0);
			psi.DialogRun(hDlg);
			psi.DialogFree(hDlg);
		}
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
