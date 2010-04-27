/**
	© 2010 Andrew Grechkin
	Source code: <http://code.google.com/p/andrew-grechkin>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef __FAR_HELPER_HPP__
#define __FAR_HELPER_HPP__

#include "win_def.h"
#include "plugin.hpp"

///======================================================================================== external
extern PluginStartupInfo	psi;
extern FarStandardFunctions fsf;

///====================================================================================== definition
#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     0

#ifndef EXP_NAME
#define EXP_NAME(p) _export p ## W
#endif

#ifdef __x86_64__
#undef _export
#define _export __declspec(dllexport)
#endif

enum		{
	MenuTitle,
	DiskTitle,
	DlgTitle,

	txtBtnOk,
	txtBtnCancel,
};

struct		InitDialogItem {
	int			Type;
	int			X1, Y1, X2, Y2;
	int			Focus;
	DWORD_PTR	Reserved;
	DWORD		Flags;
	int			DefaultButton;
	PCWSTR		Data;
};

///=================================================================================================
inline PCWSTR		GetMsg(int MsgId) {
	return	psi.GetMsg(psi.ModuleNumber, MsgId);
}
inline PCWSTR		GetDataPtr(HANDLE hDlg, size_t in) {
	return	(PCWSTR)psi.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR , in, 0);
}
inline bool			GetCheck(HANDLE hDlg, size_t in) {
	return	(bool)psi.SendDlgMessage(hDlg, DM_GETCHECK, in, 0);
}
inline void			InitDialogItems(InitDialogItem *Init, FarDialogItem *Item, int ItemsNumber) {
	for (int i = 0; i < ItemsNumber; ++i) {
		Item[i].Type = Init[i].Type;
		Item[i].X1 = Init[i].X1;
		Item[i].Y1 = Init[i].Y1;
		Item[i].X2 = Init[i].X2;
		Item[i].Y2 = Init[i].Y2;
		Item[i].Focus = Init[i].Focus;
		Item[i].Reserved = Init[i].Reserved;
		Item[i].Flags = Init[i].Flags;
		Item[i].DefaultButton = Init[i].DefaultButton;
		Item[i].MaxLen = 0;
		if ((DWORD_PTR)Init[i].Data < 2000)
			Item[i].PtrData = GetMsg((size_t)Init[i].Data);
		else
			Item[i].PtrData = Init[i].Data;
	}
}

inline void			faribox(PCWSTR text, PCWSTR tit = L"Info") {
	PCWSTR Msg[] = {tit, text, };
	psi.Message(psi.ModuleNumber, 0, NULL, Msg, sizeofa(Msg), 0);
}
inline void			farmbox(PCWSTR text, PCWSTR tit = L"Message") {
	PCWSTR Msg[] = {tit, text, L"OK", };
	psi.Message(psi.ModuleNumber, 0, NULL, Msg, sizeofa(Msg), 1);
}
inline void			farebox(PCWSTR text, PCWSTR tit = L"Error") {
	PCWSTR Msg[] = {tit, text, L"OK", };
	psi.Message(psi.ModuleNumber, FMSG_WARNING, NULL, Msg, sizeofa(Msg), 1);
}
inline void			farebox(DWORD err) {
	AutoUTF	title(L"Error: ");
	title += Num2Str((size_t)err);
	::SetLastError(err);
	PCWSTR Msg[] = {title.c_str(), L"OK", };
	psi.Message(psi.ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE, NULL, Msg, sizeofa(Msg), 1);
}
inline bool			farquestion(PCWSTR text, PCWSTR tit) {
	PCWSTR Msg[] = {tit, text, L"Cancel", L"OK", };
	return	psi.Message(psi.ModuleNumber, FMSG_WARNING, NULL, Msg, sizeofa(Msg), 2) == 1;
}

inline void			InitFSF(const PluginStartupInfo *psi) {
	::psi	= *psi;
	fsf		= *psi->FSF;
	::psi.FSF = &fsf;
}

///========================================================================================== FarDlg
class		FarDlg {
	HANDLE hDlg;
public:
	~FarDlg() {
		psi.DialogFree(hDlg);
	}
	FarDlg(): hDlg(INVALID_HANDLE_VALUE) {
		psi.DialogFree(hDlg);
	}
	int				Execute(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item,
				   int ItemsNumber, DWORD Reserved, DWORD Flags, FARWINDOWPROC DlgProc, LONG_PTR Param) {
		hDlg = psi.DialogInit(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
		return	psi.DialogRun(hDlg);
	}
	HANDLE			Handle() {
		return hDlg;
	};
	int				Check(int index) {
		return	(int)psi.SendDlgMessage(hDlg, DM_GETCHECK, index, 0);
	}
	PCWSTR			Str(int index) {
		return (PCWSTR)psi.SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, index, 0);
	}
	DWORD			Flags(int index) {
		FarDialogItem DialogItem;
		if (psi.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, index, (LONG_PTR)&DialogItem)) return DialogItem.Flags;
		return 0;
	};
	DWORD			Type(int index) {
		FarDialogItem DialogItem;
		if (psi.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, index, (LONG_PTR)&DialogItem)) return DialogItem.Type;
		return 0;
	};
	int				ListPos(int index) {
		return psi.SendDlgMessage(hDlg, DM_LISTGETCURPOS, index, 0);
	};
};

///========================================================================================== FarPnl
class		FarPnl {
	PanelInfo	m_pi;
	HANDLE		m_hPlug;
	PWSTR		m_CurDir;
	PluginPanelItem* m_ppi;

	size_t	m_CurDirSize;
	size_t	m_ppiSize;
	bool	m_Result;

	FarPnl();
public:
	~FarPnl() {
		WinMem::Free(m_CurDir);
		WinMem::Free(m_ppi);
	}
	FarPnl(HANDLE aPlugin, int cmd = FCTL_GETPANELINFO): m_hPlug(aPlugin), m_CurDir(NULL), m_ppi(NULL) {
		m_CurDirSize = m_ppiSize = 0;
		m_Result = psi.Control(aPlugin, cmd, sizeof(m_pi), (LONG_PTR) & m_pi) != 0;
	}

	bool		IsOK() {
		return	m_Result;
	}
	int			PanelType() {
		return	m_pi.PanelType;
	};
	int			Plugin() {
		return	m_pi.Plugin;
	};
	int			ItemsNumber() {
		return	m_pi.ItemsNumber;
	};
	int			SelectedItemsNumber() {
		return	m_pi.SelectedItemsNumber;
	};
	int			CurrentItem() {
		return	m_pi.CurrentItem;
	};
	int			ViewMode() const {
		return	m_pi.ViewMode;
	}
	DWORD		Flags() {
		return	m_pi.Flags;
	};
	PCWSTR		CurDir() {
		m_CurDirSize = psi.Control(m_hPlug, FCTL_GETPANELDIR, 0, 0);
		if (WinMem::Realloc(m_CurDir, m_CurDirSize)) {
			psi.Control(m_hPlug, FCTL_GETPANELDIR, m_CurDirSize, (LONG_PTR)m_CurDir);
		}
		return	m_CurDir;
	}
	PluginPanelItem& operator[](size_t index) {
		m_ppiSize = psi.Control(m_hPlug, FCTL_GETPANELITEM, index, 0);
		if (WinMem::Realloc(m_ppi, m_ppiSize)) {
			psi.Control(m_hPlug, FCTL_GETPANELITEM, index, (LONG_PTR)m_ppi);
		}
		return *m_ppi;
	}
	PluginPanelItem& Selected(size_t index) {
		m_ppiSize = psi.Control(m_hPlug, FCTL_GETSELECTEDPANELITEM, index, 0);
		if (WinMem::Realloc(m_ppi, m_ppiSize)) {
			psi.Control(m_hPlug, FCTL_GETSELECTEDPANELITEM, index, (LONG_PTR)m_ppi);
		}
		return *m_ppi;
	}
	void		StartSelection() {
		psi.Control(m_hPlug, FCTL_BEGINSELECTION, 0, 0);
	}
	void		Select(size_t index, bool in) {
		psi.Control(m_hPlug, FCTL_SETSELECTION, index, (LONG_PTR)in);
	}
	void		CommitSelection() {
		psi.Control(m_hPlug, FCTL_ENDSELECTION, 0, 0);
	}
	DWORD		Flags() const {
		return	m_pi.Flags;
	}
};

///========================================================================================== Editor
inline int			SetCursorPosition(int x, int y) {
	EditorSetPosition tmp = { -1, -1, -1, -1, -1, -1};
	tmp.CurLine = y;
	tmp.CurPos = x;
	return	psi.EditorControl(ECTL_SETPOSITION, &tmp);
}
inline int			UnselectBlock() {
	EditorSelect	tmp;
	tmp.BlockType = BTYPE_NONE;
	return	psi.EditorControl(ECTL_SELECT, &tmp);
}
inline int			DeleteString(int y) {
	if (SetCursorPosition(0, y))
		return	psi.EditorControl(ECTL_DELETESTRING, NULL);
	return	false;
}
inline int			GetString(int y, EditorGetString *str) {
	PWSTR	tmp;

	str->StringNumber = y;
	if (psi.EditorControl(ECTL_GETSTRING, str)) {
		if (WinMem::Alloc(tmp, Max(str->StringLength, 1))) {
			str->StringText = Copy(tmp, str->StringText, Max(str->StringLength, 1));
			if (WinMem::Alloc(tmp, Len(str->StringEOL))) {
				str->StringEOL = Copy(tmp, str->StringEOL);
				return	true;
			}
		}
	}
	return	false;
}
inline void 		ReleaseString(EditorGetString *str) {
	WinMem::Free(str->StringText);
	WinMem::Free(str->StringEOL);
}
inline int			InsertString(int y, PCWSTR str, int length, PCWSTR eol) {
	if (SetCursorPosition(0, y)) {
		if (psi.EditorControl(ECTL_INSERTSTRING, 0)) {
			EditorSetString tmp;
			tmp.StringNumber = y;
			tmp.StringText = str;
			tmp.StringEOL = eol;
			tmp.StringLength = length;
			return	psi.EditorControl(ECTL_SETSTRING, &tmp);
		}
	}
	return	false;
}

#endif
