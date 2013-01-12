/**
	svcmgr: Manage services
	Allow to manage windows services
	FAR3 plugin

	© 2013 Andrew Grechkin

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

#ifndef _FAR_PANELCONTROLLER_HPP_
#define _FAR_PANELCONTROLLER_HPP_

#include <libfar3/panelcontroller_i.hpp>
#include <libfar3/panel.hpp>

#include <libbase/message.hpp>

#include <panelmodel.hpp>

struct PanelActions;


struct PanelController: public Far::PanelController_i, public Base::Observer_p {
	~PanelController();

	PanelController();

	/// Far::Panel_i interface
	void GetOpenPanelInfo(OpenPanelInfo * Info) override;

	ssize_t GetFindData(GetFindDataInfo * Info) override;

	void FreeFindData(const FreeFindDataInfo * Info) override;

	ssize_t Compare(const CompareInfo * Info) override;

	ssize_t SetDirectory(const SetDirectoryInfo * Info) override;

	ssize_t ProcessEvent(const ProcessPanelEventInfo * Info) override;

	ssize_t ProcessInput(const ProcessPanelInputInfo * Info) override;

	/// Base::Observer_p interface
	void notify(const Base::Message & event);

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

	bool refresh();

	typedef void (PanelModel::* ModelFunc)(PanelModel::iterator);
	bool action_process(ModelFunc func, PCWSTR title);

	/// dialogs
	void show_dlg_edit_service(const PanelModel::iterator & it);

	void show_dlg_logon_as(Far::Panel & panel);

	void show_dlg_connection();

	void show_dlg_delete();

	/// misc
	bool set_view_mode(int mode);

	bool is_name_mode() const;

	PanelModel * m_model;
	PanelActions * actions;

	ustring PanelTitle;
	int ViewMode;
};

#endif
