/**
 *	win_com_quota
 *	@author		2010 Andrew Grechkin
 *	@classes to manipulate Quotas
 *	@link (ole32)
 **/

// include initguid always first
#include <initguid.h>

#include "win_com.h"

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
	pDQControl->Release();
}

NetQuota::NetQuota(const AutoUTF &path) :
	pDQControl(nullptr), m_path(SlashAdd(GetWord(path, PATH_SEPARATOR_C))) {
	WinCOM::init();
	if (!IsSupportQuota(m_path))
		throw ApiError(ERROR_NOT_SUPPORTED, m_path);
	CheckError(::CoCreateInstance(CLSID_DiskQuotaControl, nullptr, CLSCTX_INPROC_SERVER,
	                              IID_IDiskQuotaControl, (PVOID*)&pDQControl));
	CheckError(pDQControl->Initialize(m_path.c_str(), true));
}

IDiskQuotaControl* NetQuota::Info() const {
	return pDQControl;
}

AutoUTF NetQuota::path() const {
	return m_path;
}

//change state
void NetQuota::SetState(QuotaState in) const {
	DWORD state = 0;
	switch (in) {
		case stDisable:
			state = DISKQUOTA_STATE_DISABLED;
			break;
		case stEnable:
			state = DISKQUOTA_STATE_ENFORCE;
			break;
		case stTrack:
			state = DISKQUOTA_STATE_TRACK;
			break;
	}
	CheckError(pDQControl->SetQuotaState(in));
}

DWORD NetQuota::GetState() const {
	DWORD Result = 0;
	CheckError(pDQControl->GetQuotaState(&Result));
	return Result;
}

void NetQuota::Disable() const {
	SetState(stDisable);
}

void NetQuota::Enable() const {
	SetState(stEnable);
}

void NetQuota::Track() const {
	SetState(stTrack);
}

//check state
bool NetQuota::IsStateDisabled() const {
	return DISKQUOTA_IS_DISABLED(GetState());
}

bool NetQuota::IsStateTracked() const {
	return DISKQUOTA_IS_TRACKED(GetState());
}

bool NetQuota::IsStateEnforced() const {
	return DISKQUOTA_IS_ENFORCED(GetState());
}

bool NetQuota::IsLogLimit() const {
	return WinFlag::Check(GetLogFlags(), (DWORD)DISKQUOTA_LOGFLAG_USER_LIMIT);
}

bool NetQuota::IsLogThreshold() const {
	return WinFlag::Check(GetLogFlags(), (DWORD)DISKQUOTA_LOGFLAG_USER_THRESHOLD);
}

//change limits
void NetQuota::SetDefaultLimit(size_t in) const {
	CheckError(pDQControl->SetDefaultQuotaLimit(Mega2Bytes(in)));
}

void NetQuota::SetDefaultThreshold(size_t in) const {
	CheckError(pDQControl->SetDefaultQuotaThreshold(Mega2Bytes(in)));
}

void NetQuota::SetLogLimit(bool in) const {
	DWORD flag = GetLogFlags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_LIMIT, in);
	CheckError(pDQControl->SetQuotaLogFlags(flag));
}

void NetQuota::SetLogThreshold(bool in) const {
	DWORD flag = GetLogFlags();
	WinFlag::Switch(flag, (DWORD)DISKQUOTA_LOGFLAG_USER_THRESHOLD, in);
	CheckError(pDQControl->SetQuotaLogFlags(flag));
}

DWORD NetQuota::GetLogFlags() const {
	DWORD Result = 0;
	CheckError(pDQControl->GetQuotaLogFlags(&Result));
	return Result;
}

size_t NetQuota::GetDefaultLimit() const {
	LONGLONG Result;
	CheckError(pDQControl->GetDefaultQuotaLimit(&Result));
	return Bytes2Mega(Result);
}

size_t NetQuota::GetDefaultThreshold() const {
	LONGLONG Result;
	CheckError(pDQControl->GetDefaultQuotaThreshold(&Result));
	return Bytes2Mega(Result);
}

AutoUTF NetQuota::GetDefaultLimitText() const {
	WCHAR buf[MAX_PATH];
	CheckError(pDQControl->GetDefaultQuotaLimitText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuota::GetDefaultThresholdText() const {
	WCHAR buf[MAX_PATH];
	CheckError(pDQControl->GetDefaultQuotaThresholdText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuota::ParseState() const {
	DWORD in = GetState();
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_ENFORCE))
		return L"Quotas are enabled and the limit value is enforced. Users cannot exceed their quota limit.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_TRACK))
		return L"Quotas are enabled but the limit value is not being enforced. Users may exceed their quota limit.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_REBUILDING))
		return L"The volume is rebuilding its quota information.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_INCOMPLETE))
		return L"The volume's quota information is out of date. Quotas are probably disabled.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_DISABLED))
		return L"Quotas are not enabled on the volume.";
	return L"Unknown State";
}

bool NetQuota::IsSupportQuota(const AutoUTF &path) {
	DWORD dwFlags = 0;
	::GetVolumeInformationW(path.c_str(), nullptr, 0, nullptr, nullptr, &dwFlags, nullptr, 0);
	return WinFlag::Check(dwFlags, (DWORD)FILE_VOLUME_QUOTAS);
}

///=================================================================================== NetQuotaUsers
NetQuotaUsers::~NetQuotaUsers() {
	EnumClose();
	Clear();
}

NetQuotaUsers::NetQuotaUsers(const NetQuota &nq, bool autocache) :
	m_nq(nq), pDQControl(m_nq.Info()), pEnumDQUsers(nullptr) {
	PreCache();
	if (autocache)
		Cache();
}

void NetQuotaUsers::Cache() {
	IDiskQuotaUser *pDQUser = nullptr;
	EnumOpen();
	while (pEnumDQUsers->Next(1, &(pDQUser), nullptr) == S_OK) {
		BYTE sid[SECURITY_MAX_SID_SIZE];
		CheckError(pDQUser->GetSid(sid, sizeof(sid)));
		AutoUTF name = Sid::name(sid);
		if (!name.empty()) {
			LONGLONG used;
			LONGLONG lim;
			LONGLONG thr;
			pDQUser->GetQuotaUsed(&used);
			pDQUser->GetQuotaLimit(&lim);
			pDQUser->GetQuotaThreshold(&thr);
			QuotaInfo info(pDQUser, Bytes2Mega(used), Bytes2Mega(lim), Bytes2Mega(thr));
			Insert(name, info);
		}
	}
}

void NetQuotaUsers::Add(const AutoUTF &name, size_t lim, size_t thr) {
	if (Find(name)) {
		SetLimit(lim);
		SetThreshold(thr);
	} else {
		IDiskQuotaUser *usr = nullptr;
		CheckError(pDQControl->AddUserName(name.c_str(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &usr));
		CheckError(usr->SetQuotaLimit(Mega2Bytes(lim), true));
		CheckError(usr->SetQuotaThreshold(Mega2Bytes(thr), true));
		QuotaInfo info(usr, 0, lim, thr);
		Insert(name, info);
	}
}

void NetQuotaUsers::Del(const AutoUTF &name) {
	if (Find(name))
		Del();
	else
		throw ApiError(ERROR_NOT_FOUND);
}

void NetQuotaUsers::Del() {
	CheckError(pDQControl->DeleteUser(Value().usr));
	(Value().usr)->Release();
	Erase();
}

AutoUTF NetQuotaUsers::GetName() const {
	return Key();
}

AutoUTF NetQuotaUsers::GetUsedText() const {
	WCHAR buf[MAX_PATH];
	CheckError(Value().usr->GetQuotaUsedText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuotaUsers::GetLimitText() const {
	WCHAR buf[MAX_PATH];
	CheckError(Value().usr->GetQuotaLimitText(buf, sizeofa(buf)));
	return AutoUTF(buf);
}

AutoUTF NetQuotaUsers::GetThresholdText() const {
	WCHAR buf[MAX_PATH];
	CheckError(Value().usr->GetQuotaThresholdText(buf, sizeofa(buf)));
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
	CheckError(Value().usr->SetQuotaLimit(Mega2Bytes(in), true));
	Value().limit = in;
}

void NetQuotaUsers::SetThreshold(size_t in) {
	CheckError(Value().usr->SetQuotaThreshold(Mega2Bytes(in), true));
	Value().thres = in;
}

//private
void NetQuotaUsers::PreCache() {
	EnumClose();
	CheckError(pDQControl->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_ASYNC,
	                                       &pEnumDQUsers));
}

void NetQuotaUsers::EnumClose() {
	if (pEnumDQUsers) {
		pEnumDQUsers->Release();
		pEnumDQUsers = nullptr;
	}
}

void NetQuotaUsers::EnumOpen() {
	EnumClose();
	CheckError(pDQControl->CreateEnumUsers(nullptr, 0, DISKQUOTA_USERNAME_RESOLVE_NONE,
	                                       &pEnumDQUsers));
	Clear();
}
