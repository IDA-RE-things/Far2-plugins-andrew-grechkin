#ifndef PANEL_HPP_
#define PANEL_HPP_

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
