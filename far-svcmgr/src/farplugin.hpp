/**
	svcmgr: Manage services
	Allow to manage windows services
	FAR3 plugin

	© 2012 Andrew Grechkin

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

#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libbase/std.hpp>
#include <libbase/shared_ptr.hpp>
#include <libbase/str.hpp>
#include <libbase/logger.hpp>
#include <libext/exception.hpp>
#include <libext/service.hpp>
#include <libext/rc.hpp>

#include <libfar3/helper.hpp>

#include <vector>

typedef Far::Settings_t Settings_type;

struct FarPlugin;
extern Base::shared_ptr<FarPlugin> plugin;

///========================================================================================= Options
struct Options {
	ssize_t AddToPluginsMenu;
	ssize_t AddToDisksMenu;
	ssize_t TimeOut;
	WCHAR Prefix[32];

	Options();

	void load();

	void save() const;

	int configure();

private:
	Base::shared_ptr<Settings_type> m_settings;
	WCHAR Timeout[3];
};

///======================================================================================= FarPlugin
struct FarPlugin {
	Options options;

	FarPlugin(const PluginStartupInfo * psi);

	bool execute() const;

	void get_info(PluginInfo * pi) const;

	HANDLE open(const OpenInfo * Info);

	void close(HANDLE plugin);

	int configure();

	PCWSTR get_prefix() const;

	static GUID get_guid();

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();
};

///==================================================================================== ServicePanel
typedef void (*WinSvcFunc)(const ustring & name, Ext::RemoteConnection * conn);

struct PanelActions;

struct ServicePanel: public Far::IPanel, private Base::Uncopyable {
	static Far::IPanel * create(const OpenInfo * Info);

	void destroy();

public:
	~ServicePanel();

	// IPanel interface
	void GetOpenPanelInfo(OpenPanelInfo * Info);

	int GetFindData(GetFindDataInfo * Info);

	void FreeFindData(const FreeFindDataInfo * Info);

	int Compare(const CompareInfo * Info);

	int SetDirectory(const SetDirectoryInfo * Info);

	int ProcessEvent(const ProcessPanelEventInfo * Info);

	int ProcessKey(INPUT_RECORD rec);

private:
	ServicePanel();

	void cache();

	PCWSTR state_as_str(DWORD state) const;

	PCWSTR start_type_as_str(DWORD start) const;

	PCWSTR error_control_as_str(DWORD err) const;

	bool dlg_connection();

	bool dlg_local_connection();

	bool dlg_create_service();

	bool dlg_edit_service(Ext::WinServices::iterator & it);

	bool dlg_logon_as(Far::Panel & panel);

	bool menu_depends();

	bool menu_select_new_depend();

	bool action_process(WinSvcFunc func, PCWSTR title);

	bool del();

	bool change_logon();

	bool edit();

	bool view();

	bool pause();

	bool contin();

	bool start();

	bool stop();

	bool restart();

	ustring get_info(Ext::WinServices::const_iterator it) const;

	bool is_name_mode() const;

	WCHAR PanelTitle[64];
	Base::shared_ptr<Ext::RemoteConnection> m_conn;
	Ext::WinServices m_svcs;

	PanelActions * actions;

	bool need_recashe;
	int ViewMode;

	friend struct ProgressWindow;
};

struct PanelActions {
	typedef bool (ServicePanel::* ptrToFunc)();

	void add(WORD Key, DWORD Control, PCWSTR text, ptrToFunc func = nullptr, PCWSTR long_text = nullptr);

	KeyBarLabel * get_labels();

	bool exec_func(ServicePanel * panel, WORD Key, DWORD Control) const;

	size_t size() const;

private:
	struct KeyAction {
		FarKey Key;
		ptrToFunc Action;
	};

	std::vector<KeyBarLabel> labels;
	std::vector<KeyAction> actions;
};

///============================================================================== ProgressWindow
//template <typename Type, intptr_t (Type::*mem_func)(HANDLE, int, int)>
//intptr_t WINAPI dlg_proc_thunk(HANDLE dlg, int msg, int param1, void * param2) {
//	return (((Type*)(param2))->*mem_func)(dlg, msg, param1);
//}
//
//
//struct ProgressWindow {
//	~ProgressWindow();
//
//	ProgressWindow(const GUID & guid, const ServicePanel * panel, Far::Panel * info, PCWSTR title, WinSvcFunc func);
//
//	void set_name(size_t num, PCWSTR name);
//
//	intptr_t dlg_proc(HANDLE dlg, int msg, int param1);
//
//private:
//	const ServicePanel * m_panel;
//	Far::Panel * m_info;
//	WinSvcFunc m_func;
//	HANDLE m_dlg;
//};

#endif
