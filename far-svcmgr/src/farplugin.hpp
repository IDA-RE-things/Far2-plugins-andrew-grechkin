#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_net/service.h>
#include <libwin_net/rc.h>
#include <libwin_def/shared_ptr.h>

#ifndef FAR2
#	include <libwin_def/str.h>
#	include <API_far3/helper.hpp>
#else
#	include <libwin_def/reg.h>
#	include <API_far2/helper.hpp>
#endif

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
	Options();

	size_t AddToPluginsMenu;
	size_t AddToDisksMenu;
	size_t TimeOut;
	ustring Prefix;

#ifndef FAR2
	void load();
#else
	void load(const ustring & path);
#endif

	void get_parameters(const Far::Dialog & dlg);

	void save() const;

	size_t get_first_line() const;

	size_t get_current_line() const;

	size_t get_current_column() const;

	size_t get_total_lines() const;

	ssize_t get_block_type() const;

	void load_editor_info();

private:
#ifndef FAR2
	winstd::shared_ptr<Far::Settings_t> m_settings;
#else
	Register reg;
#endif

	EditorInfo m_ei;
	mutable size_t m_first_line;
};

struct FarPlugin {
	FarPlugin(const PluginStartupInfo * psi);

	bool Execute() const;

	void get_info(PluginInfo * pi) const;

#ifndef FAR2
	HANDLE open(const OpenInfo * Info);

	static GUID get_guid();
#else
	HANDLE open(int OpenFrom, INT_PTR Item);
#endif

	void close(HANDLE plugin);

	int configure();

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();

private:
	void delete_string(size_t & index, size_t & total, size_t & current) const;

	Options options;
};

extern winstd::shared_ptr<FarPlugin> plugin;

struct IFarPanel {
	virtual ~IFarPanel() {}
	virtual void GetOpenPluginInfo(OpenPluginInfo *Info) = 0;

	virtual int GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) = 0;
	virtual void FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber) = 0;

	virtual int Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode) = 0;
	virtual int ProcessEvent(int Event, void *Param) = 0;
	virtual int ProcessKey(int Key, unsigned int ControlState) = 0;
	virtual int SetDirectory(const WCHAR *Dir, int OpMode) = 0;
};

struct ServicePanel: public IFarPanel, private Uncopyable {
	ServicePanel():
		m_sm(&m_conn, false),
		need_recashe(true),
		ViewMode(3) {
	}
//	WinServices*		sm() {
//		return	&m_sm;
//	}

	ustring				host() const {
		return	m_conn.host();
	}
	void				Connect(PCWSTR host, PCWSTR user = nullptr, PCWSTR pass = nullptr) {
		m_conn.Open(host, user, pass);
	}

	PCWSTR				GetState(DWORD state) const {
		return	GetMsg(state + txtStopped - SERVICE_STOPPED);
	}
	PCWSTR				GetStartType(DWORD start) const {
		return	GetMsg(start + txtBoot - SERVICE_BOOT_START);
	}
	PCWSTR				GetErrorControl(DWORD err) const {
		return	GetMsg(err + txtIgnore - SERVICE_ERROR_IGNORE);
	}

	void				Cache() {
		if (need_recashe) {
//			farmbox(L"Recache");
			m_sm.cache();
		}
		need_recashe = true;
	}

	bool				DlgConnection();
	bool				DlgCreateService();
	bool				DlgEditSvc(WinServices::iterator & it);
	bool				DlgLogonAs(FarPnl & panel);
	bool				MenuDepends();
	bool				MenuSelectNewDepend();

	static IFarPanel * create_panel() {
		return new ServicePanel;
	}

	virtual void	GetOpenPluginInfo(OpenPluginInfo *Info);

	virtual int		GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	virtual void	FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber);

	virtual int		Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode);
	virtual int		ProcessEvent(int Event, void *Param);
	virtual int		ProcessKey(int Key, unsigned int ControlState);
	virtual int		SetDirectory(const WCHAR *Dir, int OpMode);

private:
	void view() const;
	void edit();
	void change_logon();
	ustring	get_info() const;


	RemoteConnection	m_conn;
	WinServices			m_sm;

	bool				need_recashe;
	int					ViewMode;
};

#endif
