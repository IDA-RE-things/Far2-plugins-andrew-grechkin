#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_net/service.h>
#include <libwin_net/rc.h>
#include <libwin_def/shared_ptr.h>

#include <API_far3/helper.hpp>
#include <libwin_def/str.h>
typedef Far::Settings_t Settings_type;

struct FarPlugin;
extern winstd::shared_ptr<FarPlugin> plugin;

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
	winstd::shared_ptr<Settings_type> m_settings;
};

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

struct ServicePanel: public Far::IPanel, private Uncopyable {
	~ServicePanel();

	void Cache();

	PCWSTR GetState(DWORD state) const;

	PCWSTR GetStartType(DWORD start) const;

	PCWSTR GetErrorControl(DWORD err) const;

	bool DlgConnection();
	bool DlgCreateService();
	bool DlgEditSvc(WinServices::iterator & it);
	bool DlgLogonAs(Far::Panel & panel);
	bool MenuDepends();
	bool MenuSelectNewDepend();

	static Far::IPanel * create_panel(const OpenInfo * Info);

	void destroy();

	void GetOpenPanelInfo(OpenPanelInfo * Info);

	int GetFindData(GetFindDataInfo * Info);

	void FreeFindData(const FreeFindDataInfo * Info);

	int Compare(const CompareInfo * Info);

	int SetDirectory(const SetDirectoryInfo * Info);

	int ProcessEvent(const ProcessPanelEventInfo * Info);

	int ProcessKey(INPUT_RECORD rec);

private:
	ServicePanel();

	void del();
	void view();
	void edit();
	void change_logon();
	ustring	get_info(WinServices::const_iterator it) const;

	WCHAR PanelTitle[64];
	RemoteConnection m_conn;
	WinServices m_svcs;

	bool need_recashe;
	int ViewMode;
};

#endif
