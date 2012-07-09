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

#include <libfar3/helper.hpp>

#include <libbase/logger.hpp>
#include <libbase/memory.hpp>

namespace Far {

	///==================================================================================== helper_t
	helper_t & helper_t::inst() {
		static helper_t ret;
		return ret;
	}

	helper_t & helper_t::init(const GUID & guid, const PluginStartupInfo * psi) {
		m_guid = guid;
		m_psi = *psi;
		m_fsf = *psi->FSF;
		m_psi.FSF = &m_fsf;
		return *this;
	}

	const GUID * helper_t::guid() const {
		return &m_guid;
	}

	const PluginStartupInfo & helper_t::psi() const {
		return m_psi;
	}

	const FarStandardFunctions & helper_t::fsf() const {
		return m_fsf;
	}

	helper_t::helper_t() {
	}


	///=============================================================================================
	void InitDialogItemsF(const InitDialogItemF * Init, FarDialogItem * Item, int ItemsNumber) {
		for (int i = 0; i < ItemsNumber; ++i) {
			Base::Memory::zero(&Item[i], sizeof(Item[i]));
			Item[i].Type = Init[i].Type;
			Item[i].X1 = Init[i].X1;
			Item[i].Y1 = Init[i].Y1;
			Item[i].X2 = Init[i].X2;
			Item[i].Y2 = Init[i].Y2;
			Item[i].Flags = Init[i].Flags;
			if ((DWORD_PTR)Init[i].Data < 2048) {
				Item[i].Data = get_msg((size_t)Init[i].Data);
			} else {
				Item[i].Data = Init[i].Data;
			}
		}
	}

	void ibox(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, };
		psi().Message(get_plugin_guid(), nullptr, 0, nullptr, Msg, sizeofa(Msg), 0);
	}

	void mbox(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(get_plugin_guid(), nullptr, 0, nullptr, Msg, sizeofa(Msg), 1);
	}

	void ebox(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, L"OK", };
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 1);
	}

	void ebox(PCWSTR msgs[], size_t size, PCWSTR help) {
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, help, msgs, size, 1);
	}

	void ebox(const Base::mstring & msg) {
		LogTrace();
		LogDebug(L"size: %Id\n", msg.size());
		PCWSTR tmp[msg.size() + 1];
		for (size_t i = 0; i < msg.size(); ++i) {
			LogDebug(L"mstr[%Id]: %s\n", i, msg[i]);
			tmp[i] = msg[i];
		}
		tmp[msg.size()] = L"OK";
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, nullptr, tmp, sizeofa(tmp), 1);
		LogTrace();
	}

//	void ebox_code(DWORD err) {
//		ustring title(L"Error: ");
//		title += Base::as_str(err);
//		::SetLastError(err);
//		PCWSTR Msg[] = {title.c_str(), L"OK", };
//		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
//	}
//
//	void ebox_code(DWORD err, PCWSTR line) {
//		ustring title(L"Error: ");
//		title += Base::as_str(err);
//		::SetLastError(err);
//		PCWSTR Msg[] = {title.c_str(), line, L"OK", };
//		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE, nullptr, Msg, sizeofa(Msg), 1);
//	}

	bool question(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text, L"OK", L"Cancel", };
		return psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING, nullptr, Msg, sizeofa(Msg), 2) == 0;
	}


	///======================================================================================= Panel
	IPanel::~IPanel() {
	}

	INT_PTR IPanel::update(bool keep_selection) const {
		return psi().PanelControl((HANDLE)this, FCTL_UPDATEPANEL, keep_selection, nullptr);
	}

	INT_PTR IPanel::redraw() const {
		return psi().PanelControl((HANDLE)this, FCTL_REDRAWPANEL, 0, nullptr);
	}

}
