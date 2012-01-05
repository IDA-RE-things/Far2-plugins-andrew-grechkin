/**
	© 2012 Andrew Grechkin
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

#ifndef __FAR3_HELPER_HPP__
#define __FAR3_HELPER_HPP__

#include <libwin_def/std.h>
#include <API_far3/plugin.hpp>
//#include "plugin.hpp"

#define MIN_FAR_VERMAJOR  3
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     2350

namespace Far {
	enum		{
		MenuTitle,
		DiskTitle,
		DlgTitle,

		txtBtnOk,
		txtBtnCancel,
	};

	///==================================================================================== helper_t
	struct helper_t {
		static helper_t & inst() {
			static helper_t ret;
			return ret;
		}

		void init(const GUID & guid, const PluginStartupInfo * psi) {
			m_guid = guid;
			m_psi = *psi;
			m_fsf = *psi->FSF;
			m_psi.FSF = &m_fsf;
		}

		const GUID * guid() const {
			return &m_guid;
		}

		const PluginStartupInfo & psi() const {
			return m_psi;
		}

		const FarStandardFunctions & fsf() const {
			return m_fsf;
		}

	private:
		helper_t() {
		}

		GUID m_guid;
		PluginStartupInfo m_psi;
		FarStandardFunctions m_fsf;
	};

	inline const GUID * guid() {
		return helper_t::inst().guid();
	}

	inline const PluginStartupInfo & psi() {
		return helper_t::inst().psi();
	}

	inline const FarStandardFunctions & fsf() {
		return helper_t::inst().fsf();
	}

	///=============================================================================================
	struct	InitDialogItemF {
		FARDIALOGITEMTYPES Type;
		int X1, Y1, X2, Y2;
		DWORD Flags;
		PCWSTR Data;
	};

	///=============================================================================================
	inline PCWSTR get_msg(int MsgId) {
		return	psi().GetMsg(guid(), MsgId);
	}

	inline void InitDialogItemsF(const InitDialogItemF *Init, FarDialogItem *Item, int ItemsNumber) {
		for (int i = 0; i < ItemsNumber; ++i) {
			WinMem::Zero(Item[i]);
			Item[i].Type = Init[i].Type;
			Item[i].X1 = Init[i].X1;
			Item[i].Y1 = Init[i].Y1;
			Item[i].X2 = Init[i].X2;
			Item[i].Y2 = Init[i].Y2;
			Item[i].Flags = Init[i].Flags;
			if ((DWORD_PTR)Init[i].Data < 2048) {
				Item[i].Data = get_msg((size_t)Init[i].Data);
			} else {
				Item[i].Data = Init[i].Data;
			}
		}
	}

	inline void faribox(PCWSTR text, PCWSTR tit = L"Info") {
		PCWSTR Msg[] = {tit, text, };
		psi().Message(guid(), nullptr, 0, nullptr, Msg, sizeofa(Msg), 0);
	}

	inline void farmbox(PCWSTR text, PCWSTR tit = L"Message") {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(guid(), nullptr, 0, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void farebox(PCWSTR text, PCWSTR tit = L"Error") {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(guid(), nullptr, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void farebox(PCWSTR msgs[], size_t size, PCWSTR help = nullptr) {
		psi().Message(guid(), nullptr, FMSG_WARNING, help, msgs, size, 1);
	}

	inline void farebox_code(DWORD err) {
		ustring	title(L"Error: ");
		title += Num2Str((size_t)err);
		::SetLastError(err);
		PCWSTR Msg[] = {title.c_str(), L"OK", };
		psi().Message(guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void farebox_code(DWORD err, PCWSTR line) {
		ustring	title(L"Error: ");
		title += Num2Str((size_t)err);
		::SetLastError(err);
		PCWSTR Msg[] = {title.c_str(), line, L"OK", };
		psi().Message(guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline bool farquestion(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, L"OK", L"Cancel", };
		return	psi().Message(guid(), nullptr, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 2) == 0;
	}

	///====================================================================================== Dialog
	struct Dialog {
		~Dialog() {
			Free();
		}

		Dialog():
			m_hndl(INVALID_HANDLE_VALUE) {
		}

		bool Init(const GUID & dguid, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item,
		         int ItemsNumber, DWORD Reserved = 0, DWORD Flags = 0, FARWINDOWPROC DlgProc = nullptr, PVOID Param = nullptr) {
			Free();
			m_hndl = psi().DialogInit(guid(), &dguid, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
			return	(m_hndl && m_hndl != INVALID_HANDLE_VALUE);
		}

		int Run() {
			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE)
				return	psi().DialogRun(m_hndl);
			return -1;
		}

		HANDLE Handle() const {
			return m_hndl;
		};

		operator HANDLE() const {
			return m_hndl;
		}

		int Check(int index) const {
			return	(int)psi().SendDlgMessage(m_hndl, DM_GETCHECK, index, 0);
		}

		bool IsChanged(int index) const {
			return !(bool)psi().SendDlgMessage(m_hndl, DM_EDITUNCHANGEDFLAG, index, nullptr);
		}

		PCWSTR Str(int index) const {
			return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, index, nullptr);
		}

		DWORD Flags(int index) {
			FarDialogItem DialogItem;
			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ?
				 DialogItem.Flags :
				 0;
		};

		DWORD Type(int index) {
			FarDialogItem DialogItem;
			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ?
				DialogItem.Type :
				0;
		};

		ssize_t get_list_position(int index) const {
			return psi().SendDlgMessage(m_hndl, DM_LISTGETCURPOS, index, 0);
		};

	private:
		void Free() {
			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
				psi().DialogFree(m_hndl);
				m_hndl = INVALID_HANDLE_VALUE;
			}
		}

		HANDLE m_hndl;
	};

	inline PCWSTR get_data_ptr(HANDLE m_hndl, size_t in) {
		return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR , in, 0);
	}

	inline bool get_checkbox(HANDLE m_hndl, size_t in) {
		return	(bool)psi().SendDlgMessage(m_hndl, DM_GETCHECK, in, 0);
	}

	///======================================================================================= Panel
	struct Panel {
		~Panel() {
			WinMem::Free(m_CurDir);
			WinMem::Free(m_ppi);
		}

		Panel(const HANDLE aPlugin, FILE_CONTROL_COMMANDS cmd = FCTL_GETPANELINFO): m_hPlug(aPlugin), m_CurDir(nullptr), m_ppi(nullptr) {
			m_CurDirSize = m_ppiSize = 0;
			m_Result = psi().PanelControl(aPlugin, cmd, sizeof(m_pi), &m_pi) != 0;
		}

		bool		IsOK() {
			return	m_Result;
		}

		int			PanelType() {
			return	m_pi.PanelType;
		};

//		int			Plugin() {
//			return	m_pi.Plugin;
//		};

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
//			m_CurDirSize = psi().PanelControl(m_hPlug, FCTL_GETPANELDIR, 0, nullptr);
//			if (WinMem::Realloc(m_CurDir, m_CurDirSize)) {
//				psi().PanelControl(m_hPlug, FCTL_GETPANELDIR, m_CurDirSize, m_CurDir);
//			}
			return	m_CurDir;
		}

		PluginPanelItem & operator [](size_t index) {
			m_ppiSize = psi().PanelControl(m_hPlug, FCTL_GETPANELITEM, index, nullptr);
			if (WinMem::Realloc(m_ppi, m_ppiSize)) {
				psi().PanelControl(m_hPlug, FCTL_GETPANELITEM, index, m_ppi);
			}
			return *m_ppi;
		}

		PluginPanelItem & Selected(size_t index) {
			m_ppiSize = psi().PanelControl(m_hPlug, FCTL_GETSELECTEDPANELITEM, index, 0);
			if (WinMem::Realloc(m_ppi, m_ppiSize)) {
				psi().PanelControl(m_hPlug, FCTL_GETSELECTEDPANELITEM, index, m_ppi);
			}
			return *m_ppi;
		}

		void		StartSelection() {
			psi().PanelControl(m_hPlug, FCTL_BEGINSELECTION, 0, nullptr);
		}

		void		Select(size_t index, bool in) {
			psi().PanelControl(m_hPlug, FCTL_SETSELECTION, index, (PVOID)in);
		}

		void		CommitSelection() {
			psi().PanelControl(m_hPlug, FCTL_ENDSELECTION, 0, nullptr);
		}

		DWORD		Flags() const {
			return	m_pi.Flags;
		}

	private:
		const HANDLE m_hPlug;
		PanelInfo m_pi;
		PWSTR m_CurDir;
		PluginPanelItem * m_ppi;

		size_t	m_CurDirSize;
		size_t	m_ppiSize;
		bool	m_Result;
	};

	///========================================================================================== Editor
	namespace Editor {
		inline ustring	get_filename() {
			WCHAR Result[psi().EditorControl(-1, ECTL_GETFILENAME, 0, nullptr) + 1];
			psi().EditorControl(-1, ECTL_GETFILENAME, 0, (void*)Result);
			return ustring(Result);
		}

		inline ustring get_string(ssize_t y) {
			EditorGetString	egs = {y};
			psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
			return ustring(egs.StringText, egs.StringLength);
		}

		inline INT_PTR set_position(ssize_t y, ssize_t x = -1) {
			EditorSetPosition esp = {y, x, -1, -1, -1, -1};
			return psi().EditorControl(-1, ECTL_SETPOSITION, 0, &esp);
		}

		inline INT_PTR set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol) {
			EditorSetString	ess = {y, size, str, eol};
			return	psi().EditorControl(-1, ECTL_SETSTRING, 0, &ess);
		}

		inline INT_PTR set_string(ssize_t y, const ustring & str, PCWSTR eol) {
			EditorSetString	ess = {y, str.size(), str.c_str(), eol};
			return	psi().EditorControl(-1, ECTL_SETSTRING, 0, &ess);
		}

		inline INT_PTR insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol) {
			if (set_position(y) && psi().EditorControl(-1, ECTL_INSERTSTRING, 0, 0)) {
				return set_string(y, str, size, eol);
			}
			return	nullptr;
		}

		inline INT_PTR del_string(ssize_t y) {
			set_position(y);
			return	psi().EditorControl(-1, ECTL_DELETESTRING, 0, nullptr);
		}

		inline INT_PTR unselect_block() {
			EditorSelect tmp = {BTYPE_NONE};
			return	psi().EditorControl(-1, ECTL_SELECT, 0, &tmp);
		}

		inline INT_PTR start_undo() {
			EditorUndoRedo eur = {EUR_BEGIN, {0}};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		inline INT_PTR stop_undo() {
			EditorUndoRedo eur = {EUR_END, {0}};
			return psi().EditorControl(-1, ECTL_UNDOREDO, 0, &eur);
		}

		inline INT_PTR redraw() {
			return	psi().EditorControl(-1, ECTL_REDRAW, 0, nullptr);
		}

		inline int get_info(EditorInfo & info) {
			return psi().EditorControl(-1, ECTL_GETINFO, 0, &info);
		}

		inline int get_string(size_t index, EditorGetString & egs) {
			egs.StringNumber = index;
			return psi().EditorControl(-1, ECTL_GETSTRING, 0, &egs);
		}
	}
}

#endif
