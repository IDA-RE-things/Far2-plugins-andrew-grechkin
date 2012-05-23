#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_net/service.h>
#include <libwin_net/rc.h>
#include <libwin_def/shared_ptr.h>

#include <API_far3/helper.hpp>
#include <libwin_def/str.h>
typedef Far::Settings_t Settings_type;

#include <vector>
using std::vector;

struct FarPlugin;
extern windef::shared_ptr<FarPlugin> plugin;

// main dialog parameters
enum {
	HEIGHT = 11,
	WIDTH = 70,

	indDelAll = 1,
	indDelRep,
	indDelWithText,
	indDelWithoutText,
	indText,
	indIsMask,
};

///========================================================================================= Options
struct Options {
	size_t AddToPluginsMenu;
	size_t AddToDisksMenu;
	size_t TimeOut;
	WCHAR Prefix[32];
	WCHAR Timeout[3];

	Options();

	void get_parameters(const Far::Dialog & dlg);

	void load();

	void save() const;

private:
	windef::shared_ptr<Settings_type> m_settings;
};

///======================================================================================= FarPlugin
struct FarPlugin {
	Options options;

	FarPlugin(const PluginStartupInfo * psi);

	bool Execute() const;

	void get_info(PluginInfo * pi) const;

	HANDLE open(const OpenInfo * Info);

	static GUID get_guid();

	void close(HANDLE plugin);

	int configure();

	PCWSTR get_prefix() const;

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();
};

///==================================================================================== ServicePanel
struct PanelActions;

struct ServicePanel: public Far::IPanel, private Uncopyable {
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

	bool dlg_edit_service(WinServices::iterator & it);

	bool dlg_logon_as(Far::Panel & panel);

	bool menu_depends();

	bool menu_select_new_depend();

	bool del();

	bool change_logon();

	bool edit();

	bool view();

	bool pause();

	bool contin();

	bool start();

	bool stop();

	bool restart();

	ustring get_info(WinServices::const_iterator it) const;

	WCHAR PanelTitle[64];
	RemoteConnection m_conn;
	WinServices m_svcs;

	PanelActions * actions;

	bool need_recashe;
	int ViewMode;
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

	vector<KeyBarLabel> labels;
	vector<KeyAction> actions;
};

#endif
