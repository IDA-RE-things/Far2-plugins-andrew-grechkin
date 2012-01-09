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

#ifndef __FAR_HELPER_HPP__
#define __FAR_HELPER_HPP__

#include <libwin_def/std.h>
#include <libwin_def/str.h>
#include "plugin.hpp"

#define MIN_FAR_VERMAJOR  2
#define MIN_FAR_VERMINOR  0
#define MIN_FAR_BUILD     1807

namespace Far {
	enum {
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

		helper_t & init(const PluginStartupInfo * psi) {
			m_psi = *psi;
			m_fsf = *psi->FSF;
			m_psi.FSF = &m_fsf;
			return *this;
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

		PluginStartupInfo m_psi;
		FarStandardFunctions m_fsf;
	};

	inline const PluginStartupInfo & psi() {
		return helper_t::inst().psi();
	}

	inline const FarStandardFunctions & fsf() {
		return helper_t::inst().fsf();
	}

	///=============================================================================================
	struct	InitDialogItemF {
		int		Type;
		int		X1, Y1, X2, Y2;
		DWORD	Flags;
		PCWSTR	Data;
	};

	///=============================================================================================
	inline PCWSTR get_msg(int MsgId) {
		return	psi().GetMsg(psi().ModuleNumber, MsgId);
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
				Item[i].PtrData = get_msg((size_t)Init[i].Data);
			} else {
				Item[i].PtrData = Init[i].Data;
			}
		}
	}

	inline void faribox(PCWSTR text, PCWSTR tit = L"Info") {
		PCWSTR Msg[] = {tit, text, };
		psi().Message(psi().ModuleNumber, 0, nullptr, Msg, sizeofa(Msg), 0);
	}

	inline void farmbox(PCWSTR text, PCWSTR tit = L"Message") {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(psi().ModuleNumber, 0, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void farebox(PCWSTR text, PCWSTR tit = L"Error") {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(psi().ModuleNumber, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void farebox(PCWSTR msgs[], size_t size, PCWSTR help = nullptr) {
		psi().Message(psi().ModuleNumber, FMSG_WARNING, help, msgs, size, 1);
	}

	inline void farebox_code(DWORD err) {
		ustring	title(L"Error: ");
		title += as_str(err);
		::SetLastError(err);
		PCWSTR Msg[] = {title.c_str(), L"OK", };
		psi().Message(psi().ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline void farebox_code(DWORD err, PCWSTR line) {
		ustring	title(L"Error: ");
		title += as_str(err);
		::SetLastError(err);
		PCWSTR Msg[] = {title.c_str(), line, L"OK", };
		psi().Message(psi().ModuleNumber, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
	}

	inline bool farquestion(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, L"OK", L"Cancel", };
		return	psi().Message(psi().ModuleNumber, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 2) == 0;
	}

	///====================================================================================== Dialog
	struct Dialog {
		~Dialog() {
			Free();
		}

		Dialog():
			m_hndl(INVALID_HANDLE_VALUE) {
		}

		bool Init(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item,
		    	int ItemsNumber, DWORD Reserved = 0, DWORD Flags = 0, FARWINDOWPROC DlgProc = nullptr, LONG_PTR Param = nullptr) {
			Free();
			m_hndl = psi().DialogInit(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
			return	(m_hndl && m_hndl != INVALID_HANDLE_VALUE);
		}

		int				Run() {
			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE)
				return	psi().DialogRun(m_hndl);
			return -1;
		}

		HANDLE			Handle() const {
			return m_hndl;
		}

		operator		HANDLE() const {
			return m_hndl;
		}

		int				Check(int index) const {
			return	(int)psi().SendDlgMessage(m_hndl, DM_GETCHECK, index, 0);
		}

		bool IsChanged(int index) const {
			return	!(bool)psi().SendDlgMessage(m_hndl, DM_EDITUNCHANGEDFLAG, index, -1);
		}

		PCWSTR Str(int index) const {
			return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, index, 0);
		}

		DWORD Flags(int index) {
			FarDialogItem DialogItem;
			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, (LONG_PTR)&DialogItem) ? DialogItem.Flags : 0;
		}

		DWORD Type(int index) {
			FarDialogItem DialogItem;
			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, (LONG_PTR)&DialogItem) ? DialogItem.Type : 0;
		}

		ssize_t get_list_position(int index) const {
			return psi().SendDlgMessage(m_hndl, DM_LISTGETCURPOS, index, 0);
		}

	private:
		void			Free() {
			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
				psi().DialogFree(m_hndl);
				m_hndl = INVALID_HANDLE_VALUE;
			}
		}

		HANDLE	m_hndl;
	};

	inline PCWSTR get_data_ptr(HANDLE m_hndl, size_t in) {
		return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR , in, 0);
	}

	inline bool get_checkbox(HANDLE m_hndl, size_t in) {
		return (bool)psi().SendDlgMessage(m_hndl, DM_GETCHECK, in, 0);
	}

	///======================================================================================= Panel
	struct IPanel {
		virtual ~IPanel() {}
		virtual void GetOpenPluginInfo(OpenPluginInfo * Info) = 0;

		virtual int GetFindData(PluginPanelItem ** pPanelItem, int * pItemsNumber, int OpMode) = 0;
		virtual void FreeFindData(PluginPanelItem * PanelItem, int ItemsNumber) = 0;

		virtual int Compare(const PluginPanelItem * Item1, const PluginPanelItem * Item2, unsigned int Mode) = 0;
		virtual int ProcessEvent(int Event, void * Param) = 0;
		virtual int ProcessKey(int Key, unsigned int ControlState) = 0;
		virtual int SetDirectory(const WCHAR * Dir, int OpMode) = 0;

		int update(bool keep_selection = true) const {
			return psi().Control((HANDLE)this, FCTL_UPDATEPANEL, keep_selection, nullptr);
		}

		int redraw() const {
			return psi().Control((HANDLE)this, FCTL_REDRAWPANEL, 0, nullptr);
		}
	};

	struct Panel {
		~Panel() {
			WinMem::Free(m_CurDir);
			WinMem::Free(m_ppi);
		}

		Panel(const HANDLE aPlugin, int cmd = FCTL_GETPANELINFO):
			m_hndl(aPlugin),
			m_ppi(nullptr),
			m_CurDir(nullptr) {
			WinMem::Zero(m_pi);
			m_Result = psi().Control(m_hndl, cmd, 0, (LONG_PTR)&m_pi);
		}

		bool is_ok() const {
			return	m_Result;
		}

		int PanelType() const {
			return	m_pi.PanelType;
		}

		size_t size()const {
			return	m_pi.ItemsNumber;
		}

		size_t selected() const {
			return	m_pi.SelectedItemsNumber;
		}

		size_t current() const {
			return	m_pi.CurrentItem;
		}

		int view_mode() const {
			return	m_pi.ViewMode;
		}

		DWORD flags() const {
			return	m_pi.Flags;
		}

		PCWSTR get_current_directory() const {
			int size = psi().Control(m_hndl, FCTL_GETPANELDIR, 0, nullptr);
			if (WinMem::Realloc(m_CurDir, size * sizeof(WCHAR), 0) && psi().Control(m_hndl, FCTL_GETPANELDIR, size, (LONG_PTR)m_CurDir)) {
				return	m_CurDir;
			}
			return L"";
		}

		const PluginPanelItem * operator [](size_t index) const {
			int m_ppiSize = psi().Control(m_hndl, FCTL_GETPANELITEM, index, nullptr);
			if (WinMem::Realloc(m_ppi, m_ppiSize)) {
				psi().Control(m_hndl, FCTL_GETPANELITEM, index, (LONG_PTR)m_ppi);
			}
			return m_ppi;
		}

		const PluginPanelItem * get_selected(size_t index) const {
			int m_ppiSize = psi().Control(m_hndl, FCTL_GETSELECTEDPANELITEM, index, 0);
			if (WinMem::Realloc(m_ppi, m_ppiSize)) {
				psi().Control(m_hndl, FCTL_GETSELECTEDPANELITEM, index, (LONG_PTR)m_ppi);
			}
			return m_ppi;
		}

		void StartSelection() {
			psi().Control(m_hndl, FCTL_BEGINSELECTION, 0, 0);
		}

		void Select(size_t index, bool in) {
			psi().Control(m_hndl, FCTL_SETSELECTION, index, (LONG_PTR)in);
		}

		void CommitSelection() {
			psi().Control(m_hndl, FCTL_ENDSELECTION, 0, 0);
		}

	private:
		const HANDLE m_hndl;
		PanelInfo m_pi;
		mutable PluginPanelItem * m_ppi;
		mutable PWSTR m_CurDir;

		int m_Result;
	};

	///========================================================================================== Editor
	namespace Editor {
		inline ustring get_filename() {
			WCHAR	Result[psi().EditorControl(ECTL_GETFILENAME, nullptr) + 1];
			psi().EditorControl(ECTL_GETFILENAME, (void*)Result);
			return	ustring(Result);
		}

		inline ustring get_string(ssize_t y) {
			EditorGetString	egs = {y};
			psi().EditorControl(ECTL_GETSTRING, &egs);
			return ustring(egs.StringText, egs.StringLength);
		}

		inline int set_position(ssize_t y, ssize_t x = -1) {
			EditorSetPosition esp = {y, x, -1, -1, -1, -1};
			return psi().EditorControl(ECTL_SETPOSITION, &esp);
		}

		inline int set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol = EMPTY_STR) {
			EditorSetString	ess = {y, str, eol, size};
			return psi().EditorControl(ECTL_SETSTRING, &ess);
		}

		inline int set_string(ssize_t y, const ustring &str, PCWSTR eol = EMPTY_STR) {
			EditorSetString	ess = {y, str.c_str(), eol, str.size()};
			return	psi().EditorControl(ECTL_SETSTRING, &ess);
		}

		inline int insert_string(int y, PCWSTR str, int size, PCWSTR eol) {
			if (set_position(y) && psi().EditorControl(ECTL_INSERTSTRING, 0)) {
				return set_string(y, str, size, eol);
			}
			return	0;
		}

		inline int del_string(ssize_t y) {
			set_position(y);
			return	psi().EditorControl(ECTL_DELETESTRING, nullptr);
		}

		inline int unselect_block() {
			EditorSelect tmp = {BTYPE_NONE};
			return	psi().EditorControl(ECTL_SELECT, &tmp);
		}

		inline int start_undo() {
			EditorUndoRedo eur = {EUR_BEGIN, {0}};
			return psi().EditorControl(ECTL_UNDOREDO, &eur);
		}

		inline int stop_undo() {
			EditorUndoRedo eur = {EUR_END, {0}};
			return psi().EditorControl(ECTL_UNDOREDO, &eur);
		}

		inline int redraw() {
			return	psi().EditorControl(ECTL_REDRAW, nullptr);
		}

		inline int get_info(EditorInfo & info) {
			return psi().EditorControl(ECTL_GETINFO, &info);
		}

		inline int get_string(size_t index, EditorGetString & egs) {
			egs.StringNumber = index;
			return psi().EditorControl(ECTL_GETSTRING, &egs);
		}
	}

}

#endif
