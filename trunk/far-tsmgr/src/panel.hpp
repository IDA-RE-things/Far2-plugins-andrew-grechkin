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

#include <libwin_net/ts.h>

#include <far/helper.h>

///======================================================================================== Messages
enum	{
	txtAddToPluginsMenu = 5,
	txtAddToDiskMenu,
	txtPluginPrefix,

	txtSelectComputer,
	txtHost,
	txtEmptyForLocal,
	txtLogin,
	txtPass,
	txtEmptyForCurrent,
	txtConnecting,

	txtSendMessage,
	txtTitle,
	txtMessage,

	txtStatus,
	txtSession,

	txtOperation,
	txtMessg,
	txtComp,
	txtTerm,
	txtDisconn,
	txtLocal,
	txtLogOff,

	txtShutdown,
	txtDlgReboot,
	txtDlgTurnOff,

	txtAreYouSure,
	txtDisconnectSession,
	txtLogoffSession,
};

///=========================================================================================== Panel
class	Panel : private Uncopyable {
	winstd::shared_ptr<RemoteConnection>	m_conn;
	WinTS			m_ts;
public:
	static PCWSTR ParseState(int st);
	static PCWSTR ParseStateFull(int st);
	static AutoUTF Info(WinTS::iterator cur);

public:
	Panel() :m_conn(new RemoteConnection) {
	}

	WinTS&	ts() {
		return	m_ts;
	}

	AutoUTF	host() const {
		return	m_conn->host();
	}

	bool	DlgConnection();
	bool	DlgMessage(DWORD id);
	bool	DlgShutdown();

	void	GetOpenPluginInfo(OpenPluginInfo *Info);

	int		GetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	void	FreeFindData(PluginPanelItem *PanelItem, int ItemsNumber);

	int		Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode);
	int		ProcessEvent(int Event, void *Param);
	int		ProcessKey(int Key, unsigned int ControlState);
	int		SetDirectory(const WCHAR *Dir, int OpMode);
};

#endif
