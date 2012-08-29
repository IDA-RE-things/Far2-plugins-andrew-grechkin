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

#include "farplugin.hpp"

//ProgressWindow::~ProgressWindow() {
//	LogTrace();
//	Far::psi().DialogFree(m_dlg);
//	m_panel->update();
//	m_panel->redraw();
//}
//
//ProgressWindow::ProgressWindow(const GUID & guid, const ServicePanel * panel, Far::Panel * info, PCWSTR title, WinSvcFunc func):
//	m_panel(panel),
//	m_info(info),
//	m_func(func)
//{
////	Far::DialogBuilder Builder = Far::get_dialog_builder(guid, title, nullptr, dlg_proc_thunk<ProgressWindow, &ProgressWindow::dlg_proc>, (void*)this);
//	FarDialogItem Items[] = {
//		{DI_DOUBLEBOX, 2, 1, 40, 4, {0}, nullptr, nullptr, 0, title, 0, nullptr},
//		{DI_TEXT, 5, 2, 0, 0, {0}, nullptr, nullptr, DIF_CENTERTEXT, m_info->get_selected(0)->CustomColumnData[1], 0, nullptr},
//	};
//	m_dlg = Far::psi().DialogInit(Far::get_plugin_guid(), &guid, -1, -1, 44, 6, nullptr,
//	                              Items, Base::lengthof(Items), 0, 0, dlg_proc_thunk<ProgressWindow, &ProgressWindow::dlg_proc>, (void*)this);
//	Far::psi().DialogRun(m_dlg);
//}
//
//void ProgressWindow::set_name(size_t /*num*/, PCWSTR /*name*/) {
//	LogTrace();
//}
//
//intptr_t ProgressWindow::dlg_proc(HANDLE dlg, int msg, int param1) {
//	static ProgressWindow * current_progress = nullptr;
//	switch (msg) {
//		case DN_INITDIALOG: {
//			current_progress = this;
//			return true;
//		}
//		case DN_ENTERIDLE: {
//			bool ret = false;
//			LogTrace();
//			try {
//				while (current_progress->m_info->selected()) {
//					Ext::WinServices::const_iterator it = current_progress->m_panel->m_svcs.find(current_progress->m_info->get_selected(0)->CustomColumnData[0]);
//					if (it != current_progress->m_panel->m_svcs.end()) {
////						Far::psi().SendDlgMessage(current_progress->m_dlg, DM_SETTEXTPTR, 1, (void*)current_progress->m_info->get_selected(0)->CustomColumnData[1]);
//						Far::psi().DefDlgProc(dlg, DM_SETTEXTPTR, 1, (void*)current_progress->m_info->get_selected(0)->CustomColumnData[1]);
//						current_progress->m_func(it->Name, current_progress->m_panel->m_conn.get());
//						current_progress->m_info->clear_selection(0);
//					}
//				}
//				ret = true;
//			} catch (Ext::AbstractError & e) {
//				Base::mstring msg;
//				e.format_error(msg);
//				Far::ebox(msg);
//			}
//			LogTrace();
//			Far::psi().SendDlgMessage(dlg, DM_CLOSE, -1, 0);
//			LogTrace();
//			return ret;
//		}
//		default: {
//			LogDebug(L"msg: %d\n", msg);
//			break;
//		}
//	}
//	return Far::psi().DefDlgProc(dlg, msg, param1, nullptr);
//}
