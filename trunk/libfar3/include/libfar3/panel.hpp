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

#ifndef _LIBFAR_PANEL_HPP_
#define _LIBFAR_PANEL_HPP_

#include <libfar3/plugin.hpp>

namespace Far {

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

}

#endif
