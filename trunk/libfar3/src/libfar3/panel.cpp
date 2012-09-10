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

#include <libfar3/panel.hpp>
#include <libfar3/helper.hpp>

#include <libbase/memory.hpp>


namespace Far {

	///======================================================================================= Panel
	Panel::~Panel() {
		Base::Memory::free(m_dir);
		Base::Memory::free(m_ppi);
	}

	Panel::Panel(const HANDLE aPlugin, FILE_CONTROL_COMMANDS cmd):
		m_hndl(aPlugin),
		m_ppi(nullptr),
		m_dir(nullptr) {
		m_pi.StructSize = sizeof(m_pi);
		m_Result = psi().PanelControl(aPlugin, cmd, sizeof(m_pi), &m_pi);
	}

	bool Panel::is_ok() const {
		return m_Result;
	}

	int Panel::PanelType() const {
		return m_pi.PanelType;
	}

	size_t Panel::size() const {
		return m_pi.ItemsNumber;
	}

	size_t Panel::selected() const {
		return m_pi.SelectedItemsNumber;
	}

	size_t Panel::current() const {
		return m_pi.CurrentItem;
	}

	int Panel::view_mode() const {
		return m_pi.ViewMode;
	}

	PANELINFOFLAGS Panel::flags() const {
		return m_pi.Flags;
	}

	PCWSTR Panel::get_current_directory() const {
		size_t size = psi().PanelControl(m_hndl, FCTL_GETPANELDIRECTORY, 0, nullptr);
		if (Base::Memory::realloc(m_dir, size)) {
			m_dir->StructSize = sizeof(*m_dir);
			if (psi().PanelControl(m_hndl, FCTL_GETPANELDIRECTORY, size, m_dir)) {
				return m_dir->Name;
			}
		}
		return L"";
	}

	const PluginPanelItem * Panel::operator [](size_t index) const {
		size_t m_ppiSize = psi().PanelControl(m_hndl, FCTL_GETPANELITEM, index, nullptr);
		if (Base::Memory::realloc(m_ppi, m_ppiSize)) {
			FarGetPluginPanelItem gpi = {m_ppiSize, m_ppi};
			psi().PanelControl(m_hndl, FCTL_GETPANELITEM, index, &gpi);
		}
		return m_ppi;
	}

	const PluginPanelItem * Panel::get_selected(size_t index) const {
		size_t m_ppiSize = psi().PanelControl(m_hndl, FCTL_GETSELECTEDPANELITEM, index, nullptr);
		if (Base::Memory::realloc(m_ppi, m_ppiSize)) {
			FarGetPluginPanelItem gpi = {m_ppiSize, m_ppi};
			psi().PanelControl(m_hndl, FCTL_GETSELECTEDPANELITEM, index, &gpi);
		}
		return m_ppi;
	}

	const PluginPanelItem * Panel::get_current() const {
		return operator [](m_pi.CurrentItem);
	}

	void Panel::StartSelection() {
		psi().PanelControl(m_hndl, FCTL_BEGINSELECTION, 0, nullptr);
	}

	void Panel::Select(size_t index, bool in) {
		psi().PanelControl(m_hndl, FCTL_SETSELECTION, index, (PVOID)in);
	}

	void Panel::clear_selection(size_t index) {
		psi().PanelControl(m_hndl, FCTL_CLEARSELECTION, index, nullptr);
	}

	void Panel::CommitSelection() {
		psi().PanelControl(m_hndl, FCTL_ENDSELECTION, 0, nullptr);
	}

}
