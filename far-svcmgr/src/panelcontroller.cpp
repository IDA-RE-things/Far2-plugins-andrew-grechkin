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

#include <panelcontroller.hpp>

#include <globalinfo.hpp>
#include <panelmodel.hpp>
#include <panelactions.hpp>

#include <lang.hpp>
#include <guid.hpp>

#include <libext/exception.hpp>
#include <libext/sid.hpp>
#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder_ex.hpp>
#include <libfar3/panel.hpp>
#include <libbase/logger.hpp>
#include <libbase/pcstr.hpp>
#include <libbase/str.hpp>


using Far::get_msg;
using Base::lengthof;


PCWSTR state_as_str(DWORD state) {
	return Far::get_msg(state + txtStopped - SERVICE_STOPPED);
}

PCWSTR start_type_as_str(DWORD start) {
	return Far::get_msg(start + txtBoot - SERVICE_BOOT_START);
}

PCWSTR error_control_as_str(DWORD err) {
	return Far::get_msg(err + txtIgnore - SERVICE_ERROR_IGNORE);
}

ustring	parse_info(const Ext::Service::Info_t & info) {
	ustring	Result;
	Result += Far::get_msg(infoServiceName);
		Result += info.name;
		Result += L"\n\n";
		Result += Far::get_msg(infoDisplayName);
		Result += info.displayName;
		Result += L"\n\n";
		Result += Far::get_msg(infoDescription);
		Result += info.description;
		Result += L"\n\n";
		Result += Far::get_msg(infoPath);
		Result += info.binaryPathName;
		Result += L"\n\n";
		Result += Far::get_msg(infoState);
		Result += state_as_str(info.status.dwCurrentState);
		Result += L"\n\n";
		Result += Far::get_msg(infoStartupType);
		Result += start_type_as_str(info.startType);
		Result += L"\n\n";
		Result += Far::get_msg(infoErrorControl);
		Result += error_control_as_str(info.errorControl);
		Result += L"\n\n";
		Result += Far::get_msg(infoOrderGroup);
		Result += info.loadOrderGroup;
		Result += L"\n\n";
		Result += Far::get_msg(infoStartName);
		Result += info.serviceStartName;
		Result += L"\n\n";
		Result += Far::get_msg(infoTag);
		Result += Base::as_str(info.tagId);
		Result += L"\n\n";
		Result += Far::get_msg(infoDependencies);
		for (size_t i = 0; i < info.dependencies.size(); ++i) {
			Result += info.dependencies[i];
			Result += L"\n               ";
		}
	return Result;
}


ssize_t svc_type_to_radio_button(DWORD svc_type) {
	ssize_t ret = 0;
	if (svc_type == SERVICE_KERNEL_DRIVER)
		ret = 0;
	else if (svc_type == SERVICE_FILE_SYSTEM_DRIVER)
		ret = 1;
	else if (svc_type == SERVICE_WIN32_OWN_PROCESS)
		ret = 2;
	else if (svc_type == SERVICE_WIN32_SHARE_PROCESS)
		ret = 3;
	return ret;
}

ssize_t svc_start_type_to_radio_button(DWORD svc_start_type) {
	return svc_start_type;
}

ssize_t svc_error_control_to_radio_button(DWORD svc_error_control) {
	return svc_error_control;
}

DWORD radio_button_to_svc_type(ssize_t btn_index) {
	DWORD ret = 0;
	switch (btn_index) {
		case 0:
			ret = SERVICE_KERNEL_DRIVER;
			break;
		case 1:
			ret = SERVICE_FILE_SYSTEM_DRIVER;
			break;
		case 2:
			ret = SERVICE_WIN32_OWN_PROCESS;
			break;
		case 3:
			ret = SERVICE_WIN32_SHARE_PROCESS;
			break;
	}
	return ret;
}


void TrunCopy(PWSTR cpDest, PCWSTR cpSrc, size_t size, size_t max_len)
{
	Base::Str::copy(cpDest, cpSrc, size);
	Far::fsf().TruncStr(cpDest, max_len);

	size_t iLen = Base::Str::length(cpDest);
	if (iLen < max_len)
	{
		::wmemset(&cpDest[iLen], L' ', max_len - iLen);
		cpDest[max_len] = L'\0';
	}
}

void ShowMessage(PCWSTR title, PCWSTR name)
{
	LogTrace();
	WCHAR TruncName[MAX_PATH];
	TrunCopy(TruncName, name, lengthof(TruncName), 60);
	PCWSTR MsgItems[] =
	{
		title,
		TruncName,
	};
	Far::psi().Message(Far::get_plugin_guid(), nullptr, 0, nullptr, MsgItems, Base::lengthof(MsgItems), 0);
}


///=================================================================================================
PanelController::~PanelController() {
	LogTrace();
	get_global_info()->unregister_observer(this);
	m_model->unregister_observer(this);
	delete actions;
	delete m_model;
}

PanelController::PanelController():
	m_model(new PanelModel),
	actions(new PanelActions),
	ViewMode(L'3')
{
	LogTrace();
	m_model->update();

	actions->add(VK_F1, SHIFT_PRESSED, L"");
	actions->add(VK_F2, SHIFT_PRESSED, L"");
	actions->add(VK_F3, 0, nullptr, &PanelController::view);
	actions->add(VK_F3, SHIFT_PRESSED, L"");
	actions->add(VK_F3, LEFT_ALT_PRESSED, L"");
	actions->add(VK_F4, 0, nullptr, &PanelController::edit);
	actions->add(VK_F4, SHIFT_PRESSED, get_msg(txtBtnCreate), &PanelController::create_service);
	actions->add(VK_F4, LEFT_ALT_PRESSED, get_msg(txtBtnLogon), &PanelController::change_logon);
	actions->add(VK_F5, 0, get_msg(txtBtnStart), &PanelController::start);
	actions->add(VK_F5, SHIFT_PRESSED, get_msg(txtBtnStartP), &PanelController::restart);
	actions->add(VK_F5, LEFT_ALT_PRESSED, get_msg(txtBtnRestrt), &PanelController::restart);
	actions->add(VK_F6, 0, get_msg(txtBtnConnct), &PanelController::change_connection);
	actions->add(VK_F6, SHIFT_PRESSED, get_msg(txtBtnLocal), &PanelController::local_connection);
	actions->add(VK_F6, LEFT_ALT_PRESSED, L"");
	actions->add(VK_F7, 0, get_msg(txtBtnPause), &PanelController::pause);
	actions->add(VK_F7, SHIFT_PRESSED, get_msg(txtBtnContin), &PanelController::contin);
	actions->add(VK_F8, 0, get_msg(txtBtnStop), &PanelController::stop);
	actions->add(VK_F8, SHIFT_PRESSED, get_msg(txtBtnDelete), &PanelController::del);

	actions->add('R', LEFT_CTRL_PRESSED, nullptr, &PanelController::refresh);
	actions->add('R', RIGHT_CTRL_PRESSED, nullptr, &PanelController::refresh);

	get_global_info()->register_observer(this);
	m_model->register_observer(this);
}

void PanelController::GetOpenPanelInfo(OpenPanelInfo * Info) {
	Info->StructSize = sizeof(*Info);
	Info->Flags = OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_USEATTRHIGHLIGHTING;
	Info->HostFile = nullptr;
	if (m_model->is_drivers())
		Info->CurDir = get_msg(txtDevices);
	else
		Info->CurDir = L"";
	Info->Format = get_global_info()->prefix;
	if (m_model->get_host().empty()) {
		PanelTitle = Base::format_str(L"%s", get_global_info()->prefix);
//		_snwprintf(PanelTitle, Base::lengthof(PanelTitle), L"%s", globalInfo->Prefix);
	} else {
		PanelTitle = Base::format_str(L"%s: %s", get_global_info()->prefix, m_model->get_host().c_str());
//		_snwprintf(PanelTitle, Base::lengthof(PanelTitle), L"%s: %s", globalInfo->Prefix, m_model->get_host().c_str());
	}
	Info->PanelTitle = PanelTitle.c_str();
	Info->StartPanelMode = ViewMode;
	Info->StartSortMode = SM_DEFAULT;
/// PanelModes
	static PCWSTR colTitles0[] = {get_msg(txtClmDisplayName), L"Info", L"Info"};
	static PCWSTR colTitles3[] = {get_msg(txtClmDisplayName), get_msg(txtClmStatus), get_msg(txtClmStart)};
	static PCWSTR colTitles4[] = {get_msg(txtClmDisplayName), get_msg(txtClmStatus)};
	static PCWSTR colTitles5[] = {nullptr, get_msg(txtClmDisplayName), get_msg(txtClmStatus), get_msg(txtClmStart), nullptr};
	static PCWSTR colTitles6[] = {get_msg(txtClmName), get_msg(txtClmDisplayName)};
	static PCWSTR colTitles7[] = {get_msg(txtClmDisplayName), get_msg(txtClmStatus), nullptr};
	static PCWSTR colTitles8[] = {get_msg(txtClmDisplayName), get_msg(txtClmLogon)};
	static PCWSTR colTitles9[] = {get_msg(txtClmName), get_msg(txtClmStatus), get_msg(txtClmDep)};
	static PanelMode CustomPanelModes[] = {
		{L"NM,C6,C7", L"0,8,8", colTitles0, L"C1", L"0", 0},
		{L"N,N,N", L"0,0,0", nullptr, L"C1", L"0", 0},
		{L"N,N", L"0,0", nullptr, L"C1", L"0", 0},
		{L"N,C2,C3", L"0,7,6", colTitles3, L"C0", L"0", 0},
		{L"N,C2", L"0,7", colTitles4, L"C0,C2", L"0,6", 0},
		{L"N,C1,C2,C3,DM", L"0,0,7,6,11", colTitles5, L"C3", L"0", PMFLAGS_FULLSCREEN},
		{L"N,C1", L"40%,0", colTitles6, L"C2,C3", L"0,0", 0},
		{L"N,C2,Z", L"40%,1,0", colTitles7, L"C3", L"0", PMFLAGS_FULLSCREEN},
		{L"N,O", L"0,40%", colTitles8, L"C0", L"0", 0},
		{L"N,C2,LN", L"0,7,3", colTitles9, L"N", L"0", 0},
	};
	Info->PanelModesArray = CustomPanelModes;
	Info->PanelModesNumber = lengthof(CustomPanelModes);
	Info->KeyBar = actions->get_titles();
}

ssize_t PanelController::GetFindData(GetFindDataInfo * Info) try {
	LogTrace();
	Info->ItemsNumber = 0;
	Info->PanelItem = nullptr;

	int i = 0;
	if (!m_model->is_drivers()) {
		++i;
		Info->ItemsNumber = m_model->size() + 1;
		Base::Memory::alloc(Info->PanelItem, sizeof(*Info->PanelItem) * Info->ItemsNumber);
		Info->PanelItem[0].FileName = Far::get_msg(txtDevices);
		Info->PanelItem[0].AlternateFileName = Far::get_msg(txtDevices);
		Info->PanelItem[0].FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	} else {
		Info->ItemsNumber = m_model->size();
		Base::Memory::alloc(Info->PanelItem, sizeof(*Info->PanelItem) * Info->ItemsNumber);
	}

	for (PanelModel::iterator it = m_model->begin(); it != m_model->end(); ++it, ++i) {
		if (is_name_mode()) {
			Info->PanelItem[i].FileName = it->name.c_str();
			Info->PanelItem[i].AlternateFileName = it->displayName.c_str();
		} else {
			Info->PanelItem[i].FileName = it->displayName.c_str();
			Info->PanelItem[i].AlternateFileName = it->displayName.c_str();
		}
		Info->PanelItem[i].Description = it->description.c_str();
		Info->PanelItem[i].Owner = it->serviceStartName.c_str();
		Info->PanelItem[i].NumberOfLinks = it->dependencies.size();
		Info->PanelItem[i].FileSize = it->tagId;

		if (it->is_disabled()) {
			Info->PanelItem[i].FileAttributes = FILE_ATTRIBUTE_HIDDEN;
		}
		PCWSTR * CustomColumnData;
		if (Base::Memory::alloc(CustomColumnData, 5 * sizeof(PCWSTR))) {
			CustomColumnData[0] = it->name.c_str();
			CustomColumnData[1] = it->displayName.c_str();
			CustomColumnData[2] = state_as_str(it->status.dwCurrentState);
			CustomColumnData[3] = start_type_as_str(it->startType);
			CustomColumnData[4] = it->binaryPathName.c_str();
			Info->PanelItem[i].CustomColumnData = CustomColumnData;
			Info->PanelItem[i].CustomColumnNumber = 5;
		}
	}
	return true;
} catch (Ext::AbstractError & e) {
	Far::ebox(e.format_error());
	return false;
}

void PanelController::FreeFindData(const FreeFindDataInfo * Info) {
	LogTrace();
	for (size_t i = 0; i < Info->ItemsNumber; ++i) {
		Base::Memory::free(Info->PanelItem[i].CustomColumnData);
	}
	Base::Memory::free_v(Info->PanelItem);
}

ssize_t PanelController::Compare(const CompareInfo * Info) {
	PanelModel::const_iterator it1 = m_model->find(Info->Item1->CustomColumnData[0]);
	PanelModel::const_iterator it2 = m_model->find(Info->Item2->CustomColumnData[0]);
	if (it1 != m_model->end() && it2 != m_model->end()) {
		if (Info->Mode == SM_NAME || Info->Mode == SM_EXT) {
			if (is_name_mode())
				return Base::Str::compare_ci(it1->name.c_str(), it2->name.c_str());
			else
				return Base::Str::compare_ci(it1->displayName.c_str(), it2->displayName.c_str());
		}
		if (Info->Mode == SM_MTIME) {
			if (it1->status.dwCurrentState == it2->status.dwCurrentState)
				return Base::Str::compare_ci(it1->displayName.c_str(), it2->displayName.c_str());
			if (it1->status.dwCurrentState < it2->status.dwCurrentState)
				return 1;
			return -1;
		}
		if (Info->Mode == SM_SIZE) {
			if (it1->startType == it2->startType)
				return Base::Str::compare_ci(it1->displayName.c_str(), it2->displayName.c_str());
			if (it1->startType < it2->startType)
				return -1;
			return 1;
		}
		return -2;
	}
	return -2;
}

ssize_t PanelController::SetDirectory(const SetDirectoryInfo * Info) try {
	LogTrace();
	if (Base::Str::compare_ci(Info->Dir, get_msg(txtDevices)) == 0) {
		m_model->set_type(Ext::Service::DRIVERS);
	} else if (Base::Str::compare_ci(Info->Dir, L"..") == 0) {
		m_model->set_type(Ext::Service::SERVICES);
	}
	return true;
} catch (Ext::AbstractError & e) {
	Far::ebox(e.format_error());
	return false;
}

ssize_t PanelController::ProcessEvent(const ProcessPanelEventInfo * Info) {
	if (Info->Event == FE_CHANGEVIEWMODE) {
		set_view_mode(Far::Panel(this).view_mode()) && update();
		return true;
	} else if (Info->Event != FE_IDLE) {
//		LogDebug(L"Some event: %d\n", Info->Event);
	}
	return false;
}

ssize_t PanelController::ProcessInput(const ProcessPanelInputInfo * Info) {
//	LogDebug(L"type: 0x%x\n", (int)rec.EventType);
	INPUT_RECORD rec = Info->Rec;
	if (rec.EventType != KEY_EVENT && rec.EventType != FARMACRO_KEY_EVENT)
		return false;

	LogDebug(L"type: 0x%x, code: 0x%x, state: 0x%x\n", (int)rec.EventType, (int)rec.Event.KeyEvent.wVirtualKeyCode, rec.Event.KeyEvent.dwControlKeyState);
	return actions->exec_func(this, rec.Event.KeyEvent.wVirtualKeyCode, rec.Event.KeyEvent.dwControlKeyState);
}


void PanelController::notify(const Base::Message & event) {
	const FarGlobalInfo * info = (const FarGlobalInfo *)event.get_data();
	if (info == get_global_info()) {
		LogTrace();
		m_model->set_wait_state(info->waitForState);
		m_model->set_wait_timeout(info->waitTimeout);
	} else {
		LogTrace();
		update();
		redraw();
	}
}


bool PanelController::view() {
	LogTrace();
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size() && info.selected() && info.get_current()->CustomColumnNumber) {
		LogDebug(L"'%s'\n", info.get_current()->CustomColumnData[0]);
		PanelModel::const_iterator it = m_model->find(info.get_current()->CustomColumnData[0]);
		if (it != m_model->end()) {
			WCHAR tmp_path[MAX_PATH] = {0};
			::GetTempPathW(Base::lengthof(tmp_path), tmp_path);
			WCHAR tmp_file[MAX_PATH] = {0};
			WCHAR pid[32];
			Base::Str::convert_num(pid, ::GetCurrentProcessId());
			::GetTempFileNameW(tmp_path, pid, 0, tmp_file);

			LogDebug(L"temp_file: '%s'\n", tmp_file);
			HANDLE hfile = ::CreateFileW(tmp_file, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			LogDebug(L"temp_file handel: %p\n", hfile);
			if (hfile != INVALID_HANDLE_VALUE) {
				ustring info(parse_info(*it));
				DWORD bytesWritten = 0;
				CheckApi(::WriteFile(hfile, info.c_str(), info.size() * sizeof(WCHAR), &bytesWritten, nullptr));
				::CloseHandle(hfile);
				LogDebug(L"start view\n");
				intptr_t ret = Far::psi().Viewer(tmp_file, nullptr, 0, 0, -1, -1,
				           VF_DELETEONLYFILEONCLOSE | VF_ENABLE_F6 | VF_DISABLEHISTORY |
				           VF_NONMODAL | VF_IMMEDIATERETURN, CP_DEFAULT);
				LogDebug(L"viewer api ret: %Id\n", ret);
			}
		}
	}
	return true;
}

bool PanelController::edit() {
	LogTrace();
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (info.size() && info.selected() && info.get_current()->CustomColumnNumber) {
		PanelModel::iterator it = m_model->find(info.get_current()->CustomColumnData[0]);
		if (it != m_model->end()) {
			show_dlg_edit_service(it);
		}
	}
	return true;
}

bool PanelController::create_service() {
	LogTrace();
	show_dlg_edit_service(m_model->end());
	return true;
}

bool PanelController::change_logon() {
	LogTrace();
	Far::Panel info(this, FCTL_GETPANELINFO);
	if (!m_model->is_drivers() && info.size() && info.selected()) {
		show_dlg_logon_as(info);
	}
	return true;
}

bool PanelController::start() {
	LogTrace();
	return action_process(&PanelModel::start, Far::get_msg(txtStartingService));
}

bool PanelController::restart() {
	LogTrace();
	return action_process(&PanelModel::restart, Far::get_msg(txtRestartingService));
}

bool PanelController::change_connection() {
	LogTrace();
	show_dlg_connection();
	return true;
}

bool PanelController::local_connection() {
	LogTrace();
	if (!m_model->get_host().empty()) {
		try {
			m_model->set_host();
		} catch (Ext::AbstractError & e) {
			Far::ebox(e.format_error());
		}
	}
	return true;
}

bool PanelController::pause() {
	LogTrace();
	return action_process(&PanelModel::pause, Far::get_msg(txtPausingService));
}

bool PanelController::contin() {
	LogTrace();
	return action_process(&PanelModel::contin, Far::get_msg(txtContinueService));
}

bool PanelController::stop() {
	LogTrace();
	return action_process(&PanelModel::stop, Far::get_msg(txtStoppingService));
}

bool PanelController::del() {
	LogTrace();
	show_dlg_delete();
	return true;
}

bool PanelController::refresh() {
	LogTrace();
	m_model->update();
	return false;
}

bool PanelController::action_process(ModelFunc func, PCWSTR title) {
	Far::Panel panel(this, FCTL_GETPANELINFO);
	if (panel.size()) {
		m_model->start_batch();
		HANDLE hScreen = Far::psi().SaveScreen(0, 0, -1, -1);
		panel.StartSelection();
		try {
			for (size_t i = panel.selected(); i; --i) {
				if (panel.get_selected(0)->CustomColumnData[0]) {
					auto it = m_model->find(panel.get_selected(0)->CustomColumnData[0]);
					if (it != m_model->end()) {
						{
							ShowMessage(title, panel.get_selected(0)->CustomColumnData[1]);
							(m_model->*func)(it);
						}
						panel.clear_selection(0);
//						Far::ibox(L"222");
					}
				}
			}
		} catch (Ext::AbstractError & e) {
			Far::ebox(e.format_error());
		}
		panel.CommitSelection();
		Far::psi().RestoreScreen(hScreen);
		m_model->stop_batch();
	}
	return true;
}

/// dialogs
void PanelController::show_dlg_edit_service(const PanelModel::iterator & it) {
	using Base::lengthof;
	using Base::Str::copy;
	using Far::get_msg;

	LogTrace();
	WCHAR name[MAX_PATH] = {0};
	WCHAR dname[MAX_PATH] = {0};
	WCHAR path[Base::MAX_PATH_LEN] = {0};
	WCHAR group[MAX_PATH] = {0};
	ssize_t svc_type = svc_type_to_radio_button(m_model->is_drivers() ? Ext::Service::KERNEL_DRIVER : Ext::Service::WIN32_OWN_PROCESS);
	ssize_t start_type = svc_start_type_to_radio_button(Ext::Service::DEMAND);
	ssize_t errorControl = svc_error_control_to_radio_button(Ext::Service::NORMAL);
	if (it != m_model->end()) {
		copy(name, it->name.c_str(), lengthof(name));
		copy(dname, it->displayName.c_str(), lengthof(dname));
		copy(path, it->binaryPathName.c_str(), lengthof(path));
		copy(group, it->loadOrderGroup.c_str(), lengthof(group));
		svc_type = svc_type_to_radio_button(it->status.dwServiceType);
		start_type = svc_start_type_to_radio_button(it->startType);
		errorControl = svc_error_control_to_radio_button(it->errorControl);
	}

	auto Builder = Far::create_dialog_builder(EditServiceDialogGuid, get_msg(it != m_model->end() ? txtDlgServiceProperties : txtDlgCreateService));
	Builder->start_column();
	Builder->add_item(Far::create_label(txtDlgName));
	Builder->add_item(Far::create_edit(name, lengthof(name), 32, L"svcmgr.Name"))->Flags |= (it != m_model->end()) ? DIF_READONLY : 0;
	Builder->add_empty_line();
	Builder->add_item(Far::create_label(txtDlgDisplayName));
	Builder->add_item(Far::create_edit(dname, lengthof(dname), 32, L"svcmgr.DName"));
	Builder->add_item(Far::create_label(txtDlgBinaryPath));
	Builder->add_item(Far::create_edit(path, lengthof(path), 32, L"svcmgr.Path"));
	Builder->add_item(Far::create_label(txtDlgGroup));
	Builder->add_item(Far::create_edit(group, lengthof(group), 32, L"svcmgr.Group"));
	Builder->add_empty_line();
	Builder->start_singlebox(32, get_msg(txtDlgServiceType), true);
	FARDIALOGITEMFLAGS fl4Dev = m_model->is_drivers() ? DIF_NONE : DIF_DISABLE;
	FARDIALOGITEMFLAGS fl4Svc = m_model->is_drivers() ? DIF_DISABLE : DIF_NONE;
	Far::AddRadioButton_t svc_types[] = {
		{txtDriver, fl4Dev},
		{txtFileSystemDriver, fl4Dev},
		{txtOwnProcess, fl4Svc},
		{txtSharedProcess, fl4Svc},
	};
	Builder->add_radiobuttons(&svc_type, lengthof(svc_types), svc_types);
	Builder->end_singlebox();
	Builder->break_column();
	Builder->add_empty_line();
	Builder->add_empty_line();
	Builder->add_empty_line();
	Builder->start_singlebox(20, Far::get_msg(txtDlgStartupType), true);
	Far::AddRadioButton_t start_types[] = {
		{txtDlgBoot, fl4Dev},
		{txtDlgSystem, DIF_NONE},
		{txtDlgAuto, DIF_NONE},
		{txtDlgManual, DIF_NONE},
		{txtDlgDisbld, DIF_NONE},
	};
	Builder->add_radiobuttons(&start_type, lengthof(start_types), start_types);
	Builder->end_singlebox();

	Builder->start_singlebox(20, Far::get_msg(txtDlgErrorControl), true);
	Far::AddRadioButton_t error_controls[] = {
		{txtDlgIgnore, DIF_NONE},
		{txtDlgNormal, DIF_NONE},
		{txtDlgSevere, DIF_NONE},
		{txtDlgCritical, DIF_NONE},
	};
	Builder->add_radiobuttons(&errorControl, lengthof(error_controls), error_controls);
	Builder->end_singlebox();
	Builder->end_column();

	Builder->add_item(Far::create_separator());
	Builder->add_OKCancel(get_msg(Far::txtBtnOk), get_msg(Far::txtBtnCancel));

	while (Builder->show()) {
		LogTrace();
		try {
			if (it != m_model->end()) {
				Ext::Service::Config_t conf;
//				conf.set_dependencies();
				conf.set_display_name(dname, it->displayName.c_str());
				conf.set_error_control((Ext::Service::Error_t)errorControl, it->errorControl);
				conf.set_group(group, it->loadOrderGroup.c_str());
				conf.set_path(path, it->binaryPathName.c_str());
				conf.set_start((Ext::Service::Start_t)start_type, it->startType);
//				conf.set_tag();
				conf.set_type((Ext::Service::Type_t)radio_button_to_svc_type(svc_type), (Ext::Service::Type_t)it->status.dwServiceType);
				m_model->set_config(it, conf);
			} else {
				Ext::Service::Create_t conf(name, path);
//				conf.set_dependencies();
				conf.set_display_name(dname);
				conf.set_error_control((Ext::Service::Error_t)errorControl);
				conf.set_group(group);
				conf.set_start((Ext::Service::Start_t)start_type);
//				conf.set_tag();
				conf.set_type((Ext::Service::Type_t)radio_button_to_svc_type(svc_type));
				m_model->add(conf);
			}
			break;
		} catch (Ext::AbstractError & e) {
			Far::ebox(e.format_error());
			continue;
		}
	}
}

void PanelController::show_dlg_logon_as(Far::Panel & panel) {
	using Base::lengthof;
	using Base::Str::compare_ci;
	using Far::get_msg;

	LogTrace();
	static PCWSTR const NetworkService = L"NT AUTHORITY\\NetworkService";
	static PCWSTR const LocalService = L"NT AUTHORITY\\LocalService";
	static PCWSTR const LocalSystem = L"LocalSystem";

	WCHAR user[MAX_PATH] = {0};
	WCHAR pass[MAX_PATH] = {0};
	static const Far::AddRadioButton_t logon_types[] = {
		{txtDlgNetworkService, DIF_NONE},
		{txtDlgLocalService, DIF_NONE},
		{txtDlgLocalSystem, DIF_NONE},
		{txtDlgUserDefined, DIF_NONE},
	};
	ssize_t logonType = 3;
//	ssize_t allowDesk = 0;

	for (size_t i = 0; i < panel.selected(); ++i) {
		if (panel.get_selected(i)->CustomColumnNumber && panel.get_selected(i)->CustomColumnData[0]) {
			auto it = m_model->find(panel.get_selected(i)->CustomColumnData[0]);
			if (it != m_model->end()) {
				Base::Str::copy(user, it->serviceStartName.c_str(), lengthof(user));
				if (compare_ci(user, NetworkService) == 0 || compare_ci(user, Ext::Sid(WinNetworkServiceSid).get_full_name().c_str()) == 0)
					logonType = 0;
				else if (compare_ci(user, LocalService) == 0 || compare_ci(user, Ext::Sid(WinLocalServiceSid).get_full_name().c_str()) == 0)
					logonType = 1;
				else if (compare_ci(user, LocalSystem) == 0 || compare_ci(user, Ext::Sid(WinLocalSystemSid).get_full_name().c_str()) == 0)
					logonType = 2;
//				allowDesk = svc.get_type() & SERVICE_INTERACTIVE_PROCESS;
				break;
			}
		}
	}
	auto Builder = Far::create_dialog_builder(LogonAsDialogGuid, Far::get_msg(txtDlgLogonAs));
	Builder->add_radiobuttons(&logonType, lengthof(logon_types), logon_types);
	Builder->start_singlebox(52);
	Builder->add_item(Far::create_label(txtLogin));
	Builder->add_item(Far::create_edit(user, lengthof(user), 49));
	Builder->add_item(Far::create_label(txtPass));
	Builder->add_item(Far::create_password(pass, lengthof(pass), 49));
//	Builder->add_checkbox(Far::get_msg(txtDlgAllowDesktop), &allowDesk);
	Builder->end_singlebox();
	Builder->add_item(Far::create_separator());
	Builder->add_OKCancel(get_msg(Far::txtBtnOk), get_msg(Far::txtBtnCancel));
	if (Builder->show()) {
		ustring	username;
		switch (logonType) {
			case 0:
				username = NetworkService;
				break;
			case 1:
				username = LocalService;
				break;
			case 2:
				username = LocalSystem;
				break;
			case 3:
				username = Base::Str::find(user, Base::PATH_SEPARATOR) ? ustring(user) : ustring(L".\\") + user;
				break;
		}
		m_model->start_batch();
		HANDLE hScreen = Far::psi().SaveScreen(0, 0, -1, -1);
		panel.StartSelection();
		try {
			for (size_t i = panel.selected(); i; --i) {
				if (panel.get_selected(0)->CustomColumnNumber && panel.get_selected(0)->CustomColumnData[0]) {
					auto it = m_model->find(panel.get_selected(0)->CustomColumnData[0]);
					if (it != m_model->end()) {
						ShowMessage(Far::get_msg(txtStartNameService), panel.get_selected(0)->CustomColumnData[1]);
						Ext::Service::Logon_t conf(username.c_str(), pass);
						m_model->set_logon(it, conf);
						panel.clear_selection(0);
					}
				}
			}
		} catch (Ext::AbstractError & e) {
			Far::ebox(e.format_error());
		}
		panel.CommitSelection();
		Far::psi().RestoreScreen(hScreen);
		m_model->stop_batch();
	}
}

void PanelController::show_dlg_connection() {
	WCHAR host[MAX_PATH] = {0};
	WCHAR user[MAX_PATH] = {0};
	WCHAR pass[MAX_PATH] = {0};
	auto Builder = Far::create_dialog_builder(ConnectionDialogGuid, Far::get_msg(txtSelectComputer));
	Builder->add_item(Far::create_label(txtHost));
	Builder->add_item(Far::create_edit(host, Base::lengthof(host), 32, L"Connect.Host"));
	Builder->add_item(Far::create_label(txtEmptyForLocal));
	Builder->add_item(Far::create_separator());
	Builder->add_item(Far::create_label(txtLogin));
	Builder->add_item(Far::create_edit(user, Base::lengthof(user), 32, L"Connect.Login"));
	Builder->add_item(Far::create_label(txtPass));
	Builder->add_item(Far::create_password(pass, Base::lengthof(pass), 32));
	Builder->add_item(Far::create_label(txtEmptyForCurrent));
	Builder->add_item(Far::create_separator());
	Builder->add_OKCancel(Far::get_msg(Far::txtBtnOk), Far::get_msg(Far::txtBtnCancel));

	while (Builder->show()) {
		try {
			m_model->set_host(host, user, pass);
			break;
		} catch (Ext::AbstractError & e) {
			Far::ebox(e.format_error());
		}
	}
}

void PanelController::show_dlg_delete() {
	if (Far::question(Far::get_msg(txtAreYouSure), Far::get_msg(txtDeleteService))) {
		Far::Panel panel(this, FCTL_GETPANELINFO);
		if (panel.size() && panel.selected()) {
			m_model->start_batch();
			HANDLE hScreen = Far::psi().SaveScreen(0, 0, -1, -1);
			panel.StartSelection();
			try {
				for (size_t i = panel.selected(); i; --i) {
					if (panel.get_selected(0)->CustomColumnNumber && panel.get_selected(0)->CustomColumnData[0]) {
						auto it = m_model->find(panel.get_selected(0)->CustomColumnData[0]);
						if (it != m_model->end()) {
							ShowMessage(Far::get_msg(txtDeletingService), panel.get_selected(0)->CustomColumnData[1]);
							m_model->del(it);
							panel.clear_selection(0);
						}
					}
				}
			} catch (Ext::AbstractError & e) {
				Far::ebox(e.format_error());
			}
			panel.CommitSelection();
			Far::psi().RestoreScreen(hScreen);
			m_model->stop_batch();
		}
	}
}

bool PanelController::set_view_mode(int mode) {
	if (ViewMode != mode) {
		ViewMode = mode;
		return true;
	}
	return false;
}

bool PanelController::is_name_mode() const {
	return ViewMode == 1 || ViewMode == 2 || ViewMode == 5 || ViewMode == 6 || ViewMode == 9;
}

///=================================================================================================
Far::PanelController_i * create_FarPanel(const OpenInfo * /*Info*/) {
	try {
		return new PanelController;
	} catch (Ext::AbstractError & e) {
		LogDebug(L"%s\n", e.what().c_str());
		Far::ebox(e.format_error());
	}
	return nullptr;
}
