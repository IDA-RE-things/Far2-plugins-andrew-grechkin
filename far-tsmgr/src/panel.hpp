/**
	tsmgr: Terminal sessions manager FAR plugin
	Allow to manage sessions on Terminal server

	© 2010 Andrew Grechkin

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

#ifndef PANEL_HPP
#define PANEL_HPP

#include "win_def.h"
#include "win_net.h"

#include "../../far/far_helper.hpp"

///======================================================================================== Messages
enum	{
	txtAddToPluginsMenu = 5,
	txtAddToDiskMenu,
	txtDisksMenuHotkey,
	txtPluginPrefix,

	txtSelectComputer,
	txtHost,
	txtEmptyForLocal,
	txtLogin,
	txtPaswd,
	txtEmptyForCurrent,
	txtConnecting,

	txtSendMessage,
	txtTitle,
	txtMessage,

	txtStatus,
	txtSession,

	txtMessg,
	txtComp,
	txtDisconn,
	txtLocal,
	txtLogOff,

	txtAreYouSure,
	txtDisconnectSession,
	txtLogoffSession,
};

///=========================================================================================== Panel
class		Panel : private Uncopyable {
	RemoteConnection	m_conn;
	WinTS				m_ts;
public:
	Panel(): m_ts(&m_conn, false) {
	}
	RemoteConnection*	conn() {
		return	&m_conn;
	}
	WinTS*		ts() {
		return	&m_ts;
	}

	AutoUTF				host() const {
		return	m_conn.host();
	}
	void				Connect(PCWSTR host, PCWSTR user = NULL, PCWSTR pass = NULL) {
		m_conn.Open(host, user, pass);
	}

	DWORD				id() const {
		return	m_ts.Key();
	}
	AutoUTF				user() const {
		return	m_ts.Value().user;
	}

	bool				DlgConnection();
	bool				DlgMessage();

	void				GetOpenPluginInfo(OpenPluginInfo *Info);

	int					GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	void				FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber);

	int					Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode);
	int					ProcessEvent(int Event, void *Param);
	int					ProcessKey(int Key, unsigned int ControlState);
	int					SetDirectory(const WCHAR *Dir, int OpMode);
};

#endif
