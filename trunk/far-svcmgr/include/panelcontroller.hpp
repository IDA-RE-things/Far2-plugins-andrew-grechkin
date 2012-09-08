#ifndef _FAR_PANELCONTROLLER_HPP_
#define _FAR_PANELCONTROLLER_HPP_

#include <libext/services.hpp>
#include <libfar3/helper.hpp>


struct PanelModel;
struct PanelActions;
struct PanelUpdater;


struct PanelController: public Far::Panel_i {
	~PanelController();

	PanelController();

	/// Far::Panel_i interface
	void GetOpenPanelInfo(OpenPanelInfo * Info);

	int GetFindData(GetFindDataInfo * Info);

	void FreeFindData(const FreeFindDataInfo * Info);

	int Compare(const CompareInfo * Info);

	int SetDirectory(const SetDirectoryInfo * Info);

	int ProcessEvent(const ProcessPanelEventInfo * Info);

	int ProcessKey(INPUT_RECORD rec);

private:
	/// actions
	bool view();

	bool edit();

	bool create_service();

	bool change_logon();

	bool start();

	bool restart();

	bool change_connection();

	bool local_connection();

	bool pause();

	bool contin();

	bool stop();

	bool del();

	/// dialogs
	bool show_dlg_connection();

	bool show_dlg_create_service();

	bool show_dlg_edit_service(const Ext::Service::Info_t & info);

	bool show_dlg_logon_as(const Ext::Service::Info_t & info);

	bool show_delete_question();

	/// misc
	bool set_view_mode(int mode);

	bool is_name_mode() const;

	PanelModel * m_model;
	PanelActions * actions;
	PanelUpdater * m_updater;

	ustring PanelTitle;
	int ViewMode;
};

#endif
