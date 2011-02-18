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
//bool		NetQuota::Add(const AutoUTF &name, LONGLONG lim, LONGLONG thr) {
//	err = E_FAIL;
//	if (pDQControl) {
//		PDISKQUOTA_USER usr = nullptr;
//		Sid sid(name, L"");
//		err = pDQControl->FindUserSid(sid, DISKQUOTA_USERNAME_RESOLVE_SYNC, &usr);
//		if (SUCCEEDED(err) && usr) {
//			usr->SetQuotaLimit(Mega2Bytes(lim), true);
//			usr->SetQuotaThreshold(Mega2Bytes(thr), true);
//			usr->Release();
//		}
//	}
//	return	SUCCEEDED(err);
//}
//HRESULT		NetQuota::UserFind(const AutoUTF &name) {
//	HRESULT err = E_FAIL;
//	if (pDQControl) {
//		Sid sid(name, L"");
////		hr = pDQControl->FindUserSid(sid.GetSid(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &pDQUser);
////		if (FAILED(hr)) pDQUser = nullptr;
//	}
//	return	err;
//}

///======================================================================================== NetQuota
NetQuota::~NetQuota() {
}

NetQuota::NetQuota(const AutoUTF &path):
		m_path(SlashAdd(GetWord(path, PATH_SEPARATOR_C))) {
	WinCOM::init();
	if (!IsSupportQuota(m_path))
		throw ApiError(ERROR_NOT_SUPPORTED, m_path);
	CheckApiError(::CoCreateInstance(CLSID_DiskQuotaControl, nullptr, CLSCTX_INPROC_SERVER,
									 IID_IDiskQuotaControl, (PVOID*)&pDQControl));
	CheckApiError(pDQControl->Initialize(m_path.c_str(), true));
}

const ComObject<IDiskQuotaControl>& NetQuota::operator->() const {
	return pDQControl;
}

AutoUTF NetQuota::path() const {
	return m_path;
}

//change state
void NetQuota::SetState(DWORD in) const {
	CheckApiError(pDQControl->SetQuotaState(in));
}

DWORD NetQuota::GetState() const {
	DWORD Result = 0;
	CheckApiError(pDQControl->GetQuotaState(&Result));
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
	return WinFlag::Check(GetLogFlags(), (DWORD)DISKQUOTA_LOGFLAG_USER_LIMIT);
}

bool NetQuota::IsLogThreshold() const {
	return WinFlag::Check(GetLogFlags(), (DWORD)DISKQUOTA_LOGFLAG_USER_THRESHOLD);
}

//change limits
void NetQuota::SetDefaultLimit(size_t in) const {
	CheckApiError(pDQControl->SetDefaultQuotaLimit(Mega2Bytes(in)));
}

void NetQuota::SetDefaultThreshold(size_t in) const {
	CheckApiError(pDQControl->SetDefaultQuotaThreshold(Mega2Bytes(in)));
}

void NetQuota::SetLogLimit(bool in) const {
	DWORD flag = GetLogFlags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_LIMIT, in);
	CheckApiError(pDQControl->SetQuotaLogFlags(flag));
}

void NetQuota::SetLogThreshold(bool in) const {
	DWORD flag = GetLogFlags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_THRESHOLD, in);
	CheckApiError(pDQControl->SetQuotaLogFlags(flag));
}

DWORD NetQuota::GetLogFlags() const {
	DWORD Result = 0;
	CheckApiError(pDQControl->GetQuotaLogFlags(&Result));
	return Result;
}

size_t NetQuota::GetDefaultLimit() const {
	LONGLONG Result;
	CheckApiError(pDQControl->GetDefaultQuotaLimit(&Result));
	return Bytes2Mega(Result);
}

size_t NetQuota::GetDefaultThreshold() const {
	LONGLONG Result;
	CheckApiError(pDQControl->GetDefaultQuotaThreshold(&Result));
	return Bytes2Mega(Result);
}

AutoUTF NetQuota::GetDefaultLimitText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(pDQControl->GetDefaultQuotaLimitText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuota::GetDefaultThresholdText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(pDQControl->GetDefaultQuotaThresholdText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuota::ParseState() const {
	DWORD in = GetState();
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_ENFORCE))
		return AutoUTF(L"Quotas are enabled and the limit value is enforced. Users cannot exceed their quota limit.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_TRACK))
		return AutoUTF(L"Quotas are enabled but the limit value is not being enforced. Users may exceed their quota limit.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_REBUILDING))
		return AutoUTF(L"The volume is rebuilding its quota information.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_INCOMPLETE))
		return AutoUTF(L"The volume's quota information is out of date. Quotas are probably disabled.");
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_DISABLED))
		return AutoUTF(L"Quotas are not enabled on the volume.");
	return AutoUTF(L"Unknown State");
}

bool NetQuota::IsSupportQuota(const AutoUTF &path) {
	DWORD dwFlags = 0;
	::GetVolumeInformationW(path.c_str(), nullptr, 0, nullptr, nullptr, &dwFlags, nullptr, 0);
	return WinFlag::Check(dwFlags, (DWORD)FILE_VOLUME_QUOTAS);
}

///=================================================================================== NetQuotaUsers
QuotaInfo::QuotaInfo(const ComObject<IDiskQuotaUser> &u) :
		usr(u) {
	LONGLONG tmp;
	u->GetQuotaUsed(&tmp);
	used = Bytes2Mega(tmp);
	u->GetQuotaLimit(&tmp);
	limit = Bytes2Mega(tmp);
	u->GetQuotaThreshold(&tmp);
	thres = Bytes2Mega(tmp);
}

QuotaInfo::QuotaInfo(const ComObject<IDiskQuotaUser> &u, size_t s, size_t l, size_t t):
		usr(u), used(s), limit(l), thres(t)  {
}

NetQuotaUsers::~NetQuotaUsers() {
	Clear();
}

NetQuotaUsers::NetQuotaUsers(const NetQuota &nq):
	m_nq(nq) {
	// precache
	ComObject<IEnumDiskQuotaUsers> qenum;
	CheckApiError(m_nq->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_ASYNC, &qenum));
}

void NetQuotaUsers::Cache() {
	Clear();
	ComObject<IEnumDiskQuotaUsers> qenum;
	CheckApiError(m_nq->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_NONE, &qenum));

	ComObject<IDiskQuotaUser> quser;
	while (qenum->Next(1, &quser, nullptr) == S_OK) {
		BYTE sid[SECURITY_MAX_SID_SIZE];
		CheckApiError(quser->GetSid(sid, sizeof(sid)));
		try {
			AutoUTF name = Sid::name(sid);
			QuotaInfo info(quser);
			Insert(name, info);
		} catch (WinError &e) {
		}
	}
}

void NetQuotaUsers::Add(const AutoUTF &name, size_t lim, size_t thr) {
	if (Find(name)) {
		SetLimit(lim);
		SetThreshold(thr);
	} else {
		ComObject<IDiskQuotaUser> usr;
		CheckApiError(m_nq->AddUserName(name.c_str(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &usr));
		CheckApiError(usr->SetQuotaLimit(Mega2Bytes(lim), true));
		CheckApiError(usr->SetQuotaThreshold(Mega2Bytes(thr), true));
		Insert(name, QuotaInfo(usr, 0, lim, thr));
	}
}

void NetQuotaUsers::Del(const AutoUTF &name) {
	if (Find(name))
		Del();
	else
		throw ApiError(ERROR_NOT_FOUND);
}

void NetQuotaUsers::Del() {
	CheckApiError(m_nq->DeleteUser(Value().usr));
	Erase();
}

AutoUTF NetQuotaUsers::GetName() const {
	return Key();
}

AutoUTF NetQuotaUsers::GetUsedText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(Value().usr->GetQuotaUsedText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuotaUsers::GetLimitText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(Value().usr->GetQuotaLimitText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuotaUsers::GetThresholdText() const {
	WCHAR buf[MAX_PATH];
	CheckApiError(Value().usr->GetQuotaThresholdText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

size_t NetQuotaUsers::GetUsed() const {
	return Value().used;
}

size_t NetQuotaUsers::GetLimit() const {
	return Value().limit;
}

size_t NetQuotaUsers::GetThreshold() const {
	return Value().thres;
}

void NetQuotaUsers::SetLimit(size_t in) {
	CheckApiError(Value().usr->SetQuotaLimit(Mega2Bytes(in), true));
	Value().limit = in;
}

void NetQuotaUsers::SetThreshold(size_t in) {
	CheckApiError(Value().usr->SetQuotaThreshold(Mega2Bytes(in), true));
	Value().thres = in;
}
