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

#include <libbase/std.hpp>
#include <libbase/mstring.hpp>

#include <libfar3/plugin.hpp>

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
		static helper_t & inst();

		helper_t & init(const GUID & guid, const PluginStartupInfo * psi);

		const GUID * guid() const;

		const PluginStartupInfo & psi() const;

		const FarStandardFunctions & fsf() const;

	private:
		helper_t();

		GUID m_guid;
		PluginStartupInfo m_psi;
		FarStandardFunctions m_fsf;
	};

	inline const GUID * get_plugin_guid() {
		return helper_t::inst().guid();
	}

	inline const PluginStartupInfo & psi() {
		return helper_t::inst().psi();
	}

	inline const FarStandardFunctions & fsf() {
		return helper_t::inst().fsf();
	}

	///=============================================================================================
	struct InitDialogItemF {
		FARDIALOGITEMTYPES Type;
		int X1, Y1, X2, Y2;
		DWORD Flags;
		PCWSTR Data;
	};

	///=============================================================================================
	PCWSTR get_msg(ssize_t MsgId);

	void InitDialogItemsF(const InitDialogItemF * Init, FarDialogItem * Item, int ItemsNumber);

	void ibox(PCWSTR text, PCWSTR tit = L"Info");

	void mbox(PCWSTR text, PCWSTR tit = L"Message");

	void ebox(PCWSTR text, PCWSTR tit = L"Error");

	void ebox(PCWSTR msgs[], size_t size, PCWSTR help = nullptr);

	void ebox(const Base::mstring & msg);

//	void ebox_code(DWORD err);
//
//	void ebox_code(DWORD err, PCWSTR line);

	bool question(PCWSTR text, PCWSTR tit);

//	///====================================================================================== Dialog
//	struct Dialog {
//		~Dialog() {
//			Free();
//		}
//
//		Dialog() :
//			m_hndl(INVALID_HANDLE_VALUE) {
//		}
//
//		bool Init(const GUID & dguid, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item, int ItemsNumber, DWORD Reserved = 0, DWORD Flags = 0, FARWINDOWPROC DlgProc = nullptr,
//		          PVOID Param = nullptr) {
//			Free();
//			m_hndl = psi().DialogInit(get_plugin_guid(), &dguid, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
//			return (m_hndl && m_hndl != INVALID_HANDLE_VALUE);
//		}
//
//		int Run() {
//			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE)
//				return psi().DialogRun(m_hndl);
//			return -1;
//		}
//
//		HANDLE Handle() const {
//			return m_hndl;
//		}
//
//		operator HANDLE() const {
//			return m_hndl;
//		}
//
//		int Check(int index) const {
//			return (int)psi().SendDlgMessage(m_hndl, DM_GETCHECK, index, 0);
//		}
//
//		bool IsChanged(int index) const {
//			return !(bool)psi().SendDlgMessage(m_hndl, DM_EDITUNCHANGEDFLAG, index, nullptr);
//		}
//
//		PCWSTR Str(int index) const {
//			return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, index, nullptr);
//		}
//
//		DWORD Flags(int index) {
//			FarDialogItem DialogItem;
//			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ? DialogItem.Flags : 0;
//		}
//
//		DWORD Type(int index) {
//			FarDialogItem DialogItem;
//			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ? DialogItem.Type : 0;
//		}
//
//		ssize_t get_list_position(int index) const {
//			return psi().SendDlgMessage(m_hndl, DM_LISTGETCURPOS, index, 0);
//		}
//
//	private:
//		void Free() {
//			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
//				psi().DialogFree(m_hndl);
//				m_hndl = INVALID_HANDLE_VALUE;
//			}
//		}
//
//		HANDLE m_hndl;
//	};
//
//	inline PCWSTR get_data_ptr(HANDLE m_hndl, size_t in) {
//		return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, in, 0);
//	}
//
//	inline bool get_checkbox(HANDLE m_hndl, size_t in) {
//		return (bool)psi().SendDlgMessage(m_hndl, DM_GETCHECK, in, 0);
//	}


	///===================================================================================== Panel_i
	struct IPanel {
		virtual ~IPanel();

		virtual void destroy() = 0;

		virtual void GetOpenPanelInfo(OpenPanelInfo * Info) = 0;

		virtual int GetFindData(GetFindDataInfo * Info) = 0;

		virtual void FreeFindData(const FreeFindDataInfo * Info) = 0;

		virtual int Compare(const CompareInfo * Info) = 0;

		virtual int SetDirectory(const SetDirectoryInfo * Info) = 0;

		virtual int ProcessEvent(const ProcessPanelEventInfo * Info) = 0;

		virtual int ProcessKey(INPUT_RECORD rec) = 0;

		INT_PTR update(bool keep_selection = true) const;

		INT_PTR redraw() const;
	};



	///======================================================================================= Panel
	struct Panel {
		~Panel();

		Panel(const HANDLE aPlugin, FILE_CONTROL_COMMANDS cmd = FCTL_GETPANELINFO);

		bool is_ok() const;

		int PanelType() const;

		size_t size() const;

		size_t selected() const;

		size_t current() const;

		int view_mode() const;

		PANELINFOFLAGS flags() const;

		PCWSTR get_current_directory() const;

		const PluginPanelItem * operator [](size_t index) const;

		const PluginPanelItem * get_selected(size_t index) const;

		const PluginPanelItem * get_current() const;

		void StartSelection();

		void Select(size_t index, bool in);

		void clear_selection(size_t index);

		void CommitSelection();

	private:
		const HANDLE m_hndl;
		PanelInfo m_pi;
		mutable PluginPanelItem * m_ppi;
		mutable FarPanelDirectory * m_dir;

		bool m_Result;
	};


	///============================================================================== ProgressWindow
	struct ProgressWindow {
		ProgressWindow(size_t /*size*/, PCWSTR /*title*/) {
		}

		void set_name(size_t /*num*/, PCWSTR /*name*/) {
		}
	private:
	};

//	inline uint64_t get_panel_settings() {
//		return psi().AdvControl(get_plugin_guid(), ACTL_GETPANELSETTINGS, 0, nullptr);
//	} GetSetting(FSSF_PANEL,L"ShowHidden")?true:false;

//	inline uint64_t get_interface_settings() {
//		return psi().AdvControl(get_plugin_guid(), ACTL_GETINTERFACESETTINGS, 0, nullptr);
//	}

	///====================================================================================== Editor
	namespace Editor {
		ssize_t get_filename(PWSTR buf, ssize_t size);

		ssize_t get_string(ssize_t y, PCWSTR & str);

		INT_PTR set_position(ssize_t y, ssize_t x = -1);

		INT_PTR set_string(ssize_t y, PCWSTR str, size_t size, PCWSTR eol);

		INT_PTR insert_string(ssize_t y, PCWSTR str, int size, PCWSTR eol);

		INT_PTR del_string(ssize_t y);

		INT_PTR unselect_block();

		INT_PTR start_undo();

		INT_PTR stop_undo();

		INT_PTR redraw();

		int get_info(EditorInfo & info);

		int get_string(size_t index, EditorGetString & egs);
	}

	///================================================================================== Settings_t
	struct Settings_t {
		~Settings_t();

		Settings_t(const GUID & guid);

		int create_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		int open_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool del(FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool del(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		size_t get(PCWSTR name, PVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		PCWSTR get(PCWSTR name, PCWSTR def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		uint64_t get(PCWSTR name, uint64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		int64_t get(PCWSTR name, int64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		uint32_t get(PCWSTR name, uint32_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		int32_t get(PCWSTR name, int32_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool get(PCWSTR name, bool def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool set(PCWSTR name, PCVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, PCWSTR value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, uint64_t value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

	private:
		HANDLE m_hndl;
	};


	///================================================================================= KeyAction_t
	typedef bool (IPanel::*PanelMemFun)();

	struct KeyAction_t {
		FarKey Key;
		PCWSTR Text;
		PCWSTR LongText;
		PanelMemFun Handler;
	};

}

#endif
