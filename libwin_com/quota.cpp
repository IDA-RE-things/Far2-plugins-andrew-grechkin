/**
 * quota
 * manipulate Quotas
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

// include initguid always first
#include <initguid.h>

#include "quota.h"
#include <libwin_net/exception.h>

#include <dskquota.h>

///===================================================================================== definitions
//bool		NetQuota::Add(const ustring &name, LONGLONG lim, LONGLONG thr) {
//	err = E_FAIL;
//	if (m_control) {
//		PDISKQUOTA_USER usr = nullptr;
//		Sid sid(name, L"");
//		err = m_control->FindUserSid(sid, DISKQUOTA_USERNAME_RESOLVE_SYNC, &usr);
//		if (SUCCEEDED(err) && usr) {
//			usr->SetQuotaLimit(Mega2Bytes(lim), true);
//			usr->SetQuotaThreshold(Mega2Bytes(thr), true);
//			usr->Release();
//		}
//	}
//	return SUCCEEDED(err);
//}
//HRESULT		NetQuota::UserFind(const ustring &name) {
//	HRESULT err = E_FAIL;
//	if (m_control) {
//		Sid sid(name, L"");
////		hr = m_control->FindUserSid(sid.GetSid(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &pDQUser);
////		if (FAILED(hr)) pDQUser = nullptr;
//	}
//	return err;
//}

///======================================================================================= QuotaInfo
QuotaInfo::QuotaInfo(const ComObject<IDiskQuotaControl> &ctrl, PCWSTR name):
	m_name(name) {
	CheckApiError(ctrl->FindUserName(name, &m_usr));
}

QuotaInfo::QuotaInfo(const ComObject<IDiskQuotaUser> &usr) :
	m_usr(usr) {
}

ustring QuotaInfo::get_name() const {
	if (m_name.empty())
		m_name = get_name(m_usr);
	return m_name;
}

ustring QuotaInfo::get_used_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_usr->GetQuotaUsedText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring QuotaInfo::get_limit_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_usr->GetQuotaLimitText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring QuotaInfo::get_threshold_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_usr->GetQuotaThresholdText(buf, sizeofa(buf)));
	return ustring(buf);
}

size_t QuotaInfo::get_used() const {
	LONGLONG tmp;
	m_usr->GetQuotaUsed(&tmp);
	return Bytes2Mega(tmp);
}

size_t QuotaInfo::get_limit() const {
	LONGLONG tmp;
	m_usr->GetQuotaLimit(&tmp);
	return Bytes2Mega(tmp);
}

size_t QuotaInfo::get_threshold() const {
	LONGLONG tmp;
	m_usr->GetQuotaThreshold(&tmp);
	return Bytes2Mega(tmp);
}

void QuotaInfo::set_limit(size_t in) {
	CheckApiError(m_usr->SetQuotaLimit(Mega2Bytes(in), true));
}

void QuotaInfo::set_threshold(size_t in) {
	CheckApiError(m_usr->SetQuotaThreshold(Mega2Bytes(in), true));
}

ustring QuotaInfo::get_name(const ComObject<IDiskQuotaUser> &usr) {
	try {
		BYTE sid[SECURITY_MAX_SID_SIZE];
		CheckApiError(usr->GetSid(sid, sizeof(sid)));
		return Sid::name(sid);
	} catch (WinError &e) {
	}
	return ustring(L"[unresolved]");
}

///======================================================================================== NetQuota
bool DiskQuota::is_supported(const ustring &path) {
	DWORD flags = 0;
	::GetVolumeInformationW(path.c_str(), nullptr, 0, nullptr, nullptr, &flags, nullptr, 0);
	return WinFlag::Check(flags, FILE_VOLUME_QUOTAS);
}

DiskQuota::~DiskQuota() {
}

DiskQuota::DiskQuota(const ustring &path):
	m_path(path) {
	WinCOM::init();
	CheckApiError(::CoCreateInstance(CLSID_DiskQuotaControl, nullptr, CLSCTX_INPROC_SERVER,
									 IID_IDiskQuotaControl, (PVOID*)&m_control));
	CheckApiError(m_control->Initialize(m_path.c_str(), true));
}

const ComObject<IDiskQuotaControl>& DiskQuota::operator->() const {
	return m_control;
}

ustring DiskQuota::path() const {
	return m_path;
}

//change state
DWORD DiskQuota::local_state_to_raw(DiskQuotaState state) const {
	switch (state) {
		case DISABLED:
			return DISKQUOTA_STATE_DISABLED;
		case TRACKED:
			return DISKQUOTA_STATE_TRACK;
		case ENABLED:
			return DISKQUOTA_STATE_ENFORCE;
	}
	return DISKQUOTA_STATE_DISABLED;
}

DiskQuota::DiskQuotaState DiskQuota::raw_state_to_local(DWORD state) const {
	switch (state) {
		case DISKQUOTA_STATE_DISABLED:
			return DISABLED;
		case DISKQUOTA_STATE_TRACK:
			return TRACKED;
		case DISKQUOTA_STATE_ENFORCE:
			return ENABLED;
	}
	return DISABLED;
}

void DiskQuota::set_state_raw(DWORD in) const {
	CheckApiError(m_control->SetQuotaState(in));
}

void DiskQuota::set_state(DiskQuotaState state) const {
	set_state_raw(local_state_to_raw(state));
}

DWORD DiskQuota::get_state_raw() const {
	DWORD Result = 0;
	CheckApiError(m_control->GetQuotaState(&Result));
	return Result;
}

DiskQuota::DiskQuotaState DiskQuota::get_state() const {
	return raw_state_to_local(get_state_raw());
}

//check state
bool DiskQuota::is_log_limit() const {
	return WinFlag::Check(get_log_flags(), DISKQUOTA_LOGFLAG_USER_LIMIT);
}

bool DiskQuota::is_log_threshold() const {
	return WinFlag::Check(get_log_flags(), DISKQUOTA_LOGFLAG_USER_THRESHOLD);
}

//change limits
void DiskQuota::set_default_limit(size_t in) const {
	CheckApiError(m_control->SetDefaultQuotaLimit(Mega2Bytes(in)));
}

void DiskQuota::set_default_threshold(size_t in) const {
	CheckApiError(m_control->SetDefaultQuotaThreshold(Mega2Bytes(in)));
}

void DiskQuota::set_log_limit(bool in) const {
	DWORD flag = get_log_flags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_LIMIT, in);
	CheckApiError(m_control->SetQuotaLogFlags(flag));
}

void DiskQuota::set_log_threshold(bool in) const {
	DWORD flag = get_log_flags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_THRESHOLD, in);
	CheckApiError(m_control->SetQuotaLogFlags(flag));
}

DWORD DiskQuota::get_log_flags() const {
	DWORD Result = 0;
	CheckApiError(m_control->GetQuotaLogFlags(&Result));
	return Result;
}

size_t DiskQuota::get_default_limit() const {
	LONGLONG Result;
	CheckApiError(m_control->GetDefaultQuotaLimit(&Result));
	return Bytes2Mega(Result);
}

size_t DiskQuota::get_default_threshold() const {
	LONGLONG Result;
	CheckApiError(m_control->GetDefaultQuotaThreshold(&Result));
	return Bytes2Mega(Result);
}

ustring DiskQuota::get_default_limit_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_control->GetDefaultQuotaLimitText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring DiskQuota::get_default_threshold_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_control->GetDefaultQuotaThresholdText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring DiskQuota::parse_state() const {
	DWORD in = get_state_raw();
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_ENFORCE))
		return ustring(L"Quotas are enabled and the limit value is enforced. Users cannot exceed their quota limit.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_TRACK))
		return ustring(L"Quotas are enabled but the limit value is not being enforced. Users may exceed their quota limit.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_REBUILDING))
		return ustring(L"The volume is rebuilding its quota information.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_INCOMPLETE))
		return ustring(L"The volume's quota information is out of date. Quotas are probably disabled.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_DISABLED))
		return ustring(L"Quotas are not enabled on the volume.");
	return ustring(L"Unknown State");
}

void DiskQuota::add(const ustring & name) {
	add_raw(name);
}

void DiskQuota::add(const ustring & name, size_t lim, size_t thr) {
	add_raw(name, lim, thr);
}

void DiskQuota::del(const ustring & name) {
	CheckApiError(m_control->DeleteUser(get_user_raw(name)));
}

ComObject<IDiskQuotaUser> DiskQuota::get_user_raw(const ustring & name) const {
	ComObject<IDiskQuotaUser> ret;
	CheckApiError(m_control->FindUserName(name.c_str(), &ret));
	return ret;
}

ComObject<IDiskQuotaUser> DiskQuota::add_raw(const ustring & name) {
	ComObject<IDiskQuotaUser> ret;
	CheckApiError(m_control->AddUserName(name.c_str(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &ret));
	return ret;
}

ComObject<IDiskQuotaUser> DiskQuota::add_raw(const ustring & name, size_t lim, size_t thr) {
	ComObject<IDiskQuotaUser> ret;
	CheckApiError(m_control->AddUserName(name.c_str(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &ret));
	CheckApiError(ret->SetQuotaLimit(Mega2Bytes(lim), true));
	CheckApiError(ret->SetQuotaThreshold(Mega2Bytes(thr), true));
	return ret;
}

void DiskQuota::del(const ComObject<IDiskQuotaUser> & user) {
	CheckApiError(m_control->DeleteUser(user));
}

///=================================================================================== NetQuotaUsers
DiskQuotaUsers::DiskQuotaUsers(const DiskQuota &nq):
	m_nq(nq) {
	{ // precache
		ComObject<IEnumDiskQuotaUsers> qenum;
		CheckApiError(m_nq->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_ASYNC, &qenum));
	}
}

void DiskQuotaUsers::cache() {
	ComObject<IEnumDiskQuotaUsers> qenum;
	CheckApiError(m_nq->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_NONE, &qenum));

	clear();

	ComObject<IDiskQuotaUser> quser;
	while (qenum->Next(1, &quser, nullptr) == S_OK) {
		try {
			QuotaInfo info(quser);
			this->insert(value_type(info.get_name(), info));
		} catch (WinError &e) {
		}
	}
}

void DiskQuotaUsers::add(const ustring & name, size_t lim, size_t thr) {
	DiskQuotaUsers::iterator it = find(name);
	if (it != end()) {
		it->second.set_limit(lim);
		it->second.set_threshold(thr);
	} else {
		ComObject<IDiskQuotaUser> usr(m_nq.add_raw(name, lim, thr));
		insert(value_type(name, QuotaInfo(usr)));
	}
}

void DiskQuotaUsers::del(const ustring & name) {
	DiskQuotaUsers::iterator it = find(name);
	if (it != end()) {
		m_nq.del(it->second.m_usr);
		erase(it);
	} else
		CheckApiError(ERROR_NOT_FOUND);
}
