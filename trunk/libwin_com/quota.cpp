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

///======================================================================================== NetQuota
NetQuota::~NetQuota() {
}

NetQuota::NetQuota(const ustring &path):
	m_path(path) {
	WinCOM::init();
	CheckApiError(::CoCreateInstance(CLSID_DiskQuotaControl, nullptr, CLSCTX_INPROC_SERVER,
									 IID_IDiskQuotaControl, (PVOID*)&m_control));
	CheckApiError(m_control->Initialize(m_path.c_str(), true));
}

const ComObject<IDiskQuotaControl>& NetQuota::operator->() const {
	return m_control;
}

ustring NetQuota::path() const {
	return m_path;
}

//change state
void NetQuota::SetState(DWORD in) const {
	CheckApiError(m_control->SetQuotaState(in));
}

DWORD NetQuota::GetState() const {
	DWORD Result = 0;
	CheckApiError(m_control->GetQuotaState(&Result));
	return Result;
}

void NetQuota::Disable() const {
	SetState(DISKQUOTA_STATE_DISABLED);
}

void NetQuota::Enable() const {
	SetState(DISKQUOTA_STATE_ENFORCE);
}

void NetQuota::Track() const {
	SetState(DISKQUOTA_STATE_TRACK);
}

//check state
bool NetQuota::IsDisabled() const {
	return DISKQUOTA_IS_DISABLED(GetState());
}

bool NetQuota::IsEnabled() const {
	return DISKQUOTA_IS_ENFORCED(GetState());
}

bool NetQuota::IsTracked() const {
	return DISKQUOTA_IS_TRACKED(GetState());
}

bool NetQuota::IsLogLimit() const {
	return WinFlag::Check(GetLogFlags(), DISKQUOTA_LOGFLAG_USER_LIMIT);
}

bool NetQuota::IsLogThreshold() const {
	return WinFlag::Check(GetLogFlags(), DISKQUOTA_LOGFLAG_USER_THRESHOLD);
}

//change limits
void NetQuota::SetDefaultLimit(size_t in) const {
	CheckApiError(m_control->SetDefaultQuotaLimit(Mega2Bytes(in)));
}

void NetQuota::SetDefaultThreshold(size_t in) const {
	CheckApiError(m_control->SetDefaultQuotaThreshold(Mega2Bytes(in)));
}

void NetQuota::SetLogLimit(bool in) const {
	DWORD flag = GetLogFlags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_LIMIT, in);
	CheckApiError(m_control->SetQuotaLogFlags(flag));
}

void NetQuota::SetLogThreshold(bool in) const {
	DWORD flag = GetLogFlags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_THRESHOLD, in);
	CheckApiError(m_control->SetQuotaLogFlags(flag));
}

DWORD NetQuota::GetLogFlags() const {
	DWORD Result = 0;
	CheckApiError(m_control->GetQuotaLogFlags(&Result));
	return Result;
}

size_t NetQuota::GetDefaultLimit() const {
	LONGLONG Result;
	CheckApiError(m_control->GetDefaultQuotaLimit(&Result));
	return Bytes2Mega(Result);
}

size_t NetQuota::GetDefaultThreshold() const {
	LONGLONG Result;
	CheckApiError(m_control->GetDefaultQuotaThreshold(&Result));
	return Bytes2Mega(Result);
}

ustring NetQuota::GetDefaultLimitText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_control->GetDefaultQuotaLimitText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring NetQuota::GetDefaultThresholdText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_control->GetDefaultQuotaThresholdText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring NetQuota::ParseState() const {
	DWORD in = GetState();
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

ComObject<IDiskQuotaUser> NetQuota::Add(const ustring &name, size_t lim, size_t thr) {
	ComObject<IDiskQuotaUser> usr(add_user(name));
	CheckApiError(usr->SetQuotaLimit(Mega2Bytes(lim), true));
	CheckApiError(usr->SetQuotaThreshold(Mega2Bytes(thr), true));
	return usr;
}

void NetQuota::Del(const ustring &name) {
	CheckApiError(m_control->DeleteUser(get_user(name)));
}

bool NetQuota::IsSupported(const ustring &path) {
	DWORD flags = 0;
	::GetVolumeInformationW(path.c_str(), nullptr, 0, nullptr, nullptr, &flags, nullptr, 0);
	return WinFlag::Check(flags, FILE_VOLUME_QUOTAS);
}

ComObject<IDiskQuotaUser> NetQuota::get_user(const ustring &name) const {
	ComObject<IDiskQuotaUser> ret;
	CheckApiError(m_control->FindUserName(name.c_str(), &ret));
	return ret;
}

ComObject<IDiskQuotaUser> NetQuota::add_user(const ustring &name) const {
	ComObject<IDiskQuotaUser> ret;
	CheckApiError(m_control->AddUserName(name.c_str(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &ret));
	return ret;
}

///=================================================================================== NetQuotaUsers
QuotaInfo::QuotaInfo(const ComObject<IDiskQuotaControl> &c, PCWSTR name) {
	CheckApiError(c->FindUserName(name, &m_usr));
}

QuotaInfo::QuotaInfo(const ComObject<IDiskQuotaUser> &u) :
	m_usr(u) {
}

ustring QuotaInfo::name() const {
	return m_name;
}

ustring QuotaInfo::used_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_usr->GetQuotaUsedText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring QuotaInfo::limit_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_usr->GetQuotaLimitText(buf, sizeofa(buf)));
	return ustring(buf);
}

ustring QuotaInfo::threshold_text() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(m_usr->GetQuotaThresholdText(buf, sizeofa(buf)));
	return ustring(buf);
}

size_t QuotaInfo::used() const {
	LONGLONG tmp;
	m_usr->GetQuotaUsed(&tmp);
	return Bytes2Mega(tmp);
}

size_t QuotaInfo::limit() const {
	LONGLONG tmp;
	m_usr->GetQuotaLimit(&tmp);
	return Bytes2Mega(tmp);
}

size_t QuotaInfo::threshold() const {
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

NetQuotaUsers::NetQuotaUsers(const NetQuota &nq):
	m_nq(nq) {
	// precache
	ComObject<IEnumDiskQuotaUsers> qenum;
	CheckApiError(m_nq->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_ASYNC, &qenum));
}

void NetQuotaUsers::Cache() {
	ComObject<IEnumDiskQuotaUsers> qenum;
	CheckApiError(m_nq->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_NONE, &qenum));
	clear();

	ComObject<IDiskQuotaUser> quser;
	while (qenum->Next(1, &quser, nullptr) == S_OK) {
		BYTE sid[SECURITY_MAX_SID_SIZE];
		CheckApiError(quser->GetSid(sid, sizeof(sid)));
		try {
			ustring name = Sid::name(sid);
			QuotaInfo info(quser);
			insert(value_type(name, info));
		} catch (WinError &e) {
		}
	}
}

void NetQuotaUsers::Add(const ustring &name, size_t lim, size_t thr) {
	NetQuotaUsers::iterator it = find(name);
	if (it != end()) {
		it->second.set_limit(lim);
		it->second.set_threshold(thr);
	} else {
		ComObject<IDiskQuotaUser> usr(m_nq.Add(name, lim, thr));
		insert(value_type(name, QuotaInfo(usr)));
	}
}

void NetQuotaUsers::Del(const ustring &name) {
	if (find(name) != end())
		; //		Del();
	else
		CheckApiError(ERROR_NOT_FOUND);
}
