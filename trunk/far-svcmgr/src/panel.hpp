#ifndef _FAR_PANEL_HPP_
#define _FAR_PANEL_HPP_

#include <libfar3/helper.hpp>

#include <libext/service.hpp>
#include <libext/rc.hpp>

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
