/** win_net_quota
 *	@author GrAnD, 2008
 *	@classes to manipulate Quotas
 *	@link (ole32)
**/

// include initguid always first
#include <initguid.h>

#include "win_com.h"

#include <dskquota.h>

///===================================================================================== definitions
struct QuotaInfo {
	IDiskQuotaUser *usr;
	size_t		limit;
	size_t		thres;
};

///================================================================================== NetQuota::Impl
class		NetQuota::Impl : public WinErrorCheck {
	IDiskQuotaControl	*pDQControl;
//	IDiskQuotaUser		*pDQUser;
	AutoUTF			path_;

	bool			GetState(DWORD &out) const;

public:
	~Impl();
	Impl(const AutoUTF &path);
	IDiskQuotaControl*	GetDQC() const;

	//change state
	bool			SetState(QuotaState in) const;

	//check state
	bool			IsStateDisabled(void) const;
	bool			IsStateTracked(void) const;
	bool			IsStateEnforced(void) const;

	//change limits
	bool			SetLogLimit(bool in = true) const;
	bool			SetLogTreshold(bool in = true) const;
	bool			SetDefaultLimit(size_t in = 0) const;
	bool			SetDefaultThreshold(size_t in = 0) const;

	AutoUTF			path() const {
		return	path_;
	}
	DWORD			GetState() const;
	DWORD			GetLogFlags() const;
	size_t			GetDefaultLimit() const;
	size_t			GetDefaultThreshold() const;
	AutoUTF			GetDefaultLimitText() const;
	AutoUTF			GetDefaultThresholdText() const;

	AutoUTF			ParseState() const {
		return	ParseState(GetState());
	}
	static AutoUTF	ParseState(DWORD in);
	static bool		IsSupport(const AutoUTF &path);

	/*
		bool			Add(const AutoUTF &name, LONGLONG lim = 0, LONGLONG thr = 0);
		HRESULT			UserFind(const AutoUTF &name);
	*/
};

bool		NetQuota::Impl::GetState(DWORD &out) const {
	err(E_FAIL);
	out = 0;
	if (pDQControl)
		err(pDQControl->GetQuotaState(&out));
	return	SUCCEEDED(err());
}

NetQuota::Impl::~Impl() {
	if (pDQControl)
		pDQControl->Release();
}
NetQuota::Impl::Impl(const AutoUTF &path): pDQControl(null_ptr), path_(SlashAdd(path)) {
	if (!WinCOM::IsOK())
		throw;	// COM ole not cannot init
	err(ERROR_NOT_SUPPORTED);
	if (IsSupport(path_)) {
		err(::CoCreateInstance(CLSID_DiskQuotaControl,
							   null_ptr,
							   CLSCTX_INPROC_SERVER,
							   IID_IDiskQuotaControl,
							   (PVOID*) & pDQControl));
		if (SUCCEEDED(err())) {
			err(pDQControl->Initialize(path_.c_str(), true));
		}
	}
}
IDiskQuotaControl* NetQuota::Impl::GetDQC() const {
	return	pDQControl;
}

bool		NetQuota::Impl::SetState(QuotaState in) const {
	err(E_FAIL);
	DWORD	dwState = 0;
	switch (in) {
		case stDisable:
			dwState = DISKQUOTA_STATE_DISABLED;
			break;
		case stEnable:
			dwState = DISKQUOTA_STATE_ENFORCE;
			break;
		case stTrack:
			dwState = DISKQUOTA_STATE_TRACK;
			break;
	}
	if (pDQControl) {
		err(pDQControl->SetQuotaState(dwState));
	}
	return	SUCCEEDED(err());
}

bool		NetQuota::Impl::IsStateDisabled() const {
	return	(DISKQUOTA_IS_DISABLED(GetState()));
}
bool		NetQuota::Impl::IsStateTracked() const {
	return	(DISKQUOTA_IS_TRACKED(GetState()));
}
bool		NetQuota::Impl::IsStateEnforced() const {
	return	(DISKQUOTA_IS_ENFORCED(GetState()));
}

bool		NetQuota::Impl::SetLogLimit(bool in) const {
	err(E_FAIL);
	if (pDQControl) {
		if (in)
			err(pDQControl->SetQuotaLogFlags(GetLogFlags() | DISKQUOTA_LOGFLAG_USER_LIMIT));
		else
			err(pDQControl->SetQuotaLogFlags(GetLogFlags() & ~DISKQUOTA_LOGFLAG_USER_LIMIT));
	}
	return	SUCCEEDED(err());
}
bool		NetQuota::Impl::SetLogTreshold(bool in) const {
	err(E_FAIL);
	if (pDQControl) {
		if (in)
			err(pDQControl->SetQuotaLogFlags(GetLogFlags() | DISKQUOTA_LOGFLAG_USER_THRESHOLD));
		else
			err(pDQControl->SetQuotaLogFlags(GetLogFlags() & ~DISKQUOTA_LOGFLAG_USER_THRESHOLD));
	}
	return	SUCCEEDED(err());
}
bool		NetQuota::Impl::SetDefaultLimit(size_t in) const {
	err(E_FAIL);
	if (pDQControl)
		err(pDQControl->SetDefaultQuotaLimit(Mega2Bytes(in)));
	return	SUCCEEDED(err());
}
bool		NetQuota::Impl::SetDefaultThreshold(size_t in) const {
	err(E_FAIL);
	if (pDQControl)
		err(pDQControl->SetDefaultQuotaThreshold(Mega2Bytes(in)));
	return	SUCCEEDED(err());
}

DWORD		NetQuota::Impl::GetState() const {
	DWORD	Result = 0;
	GetState(Result);
	return	Result;
}
DWORD		NetQuota::Impl::GetLogFlags() const {
	DWORD	Result = 0;
	err(E_FAIL);
	if (pDQControl)
		err(pDQControl->GetQuotaLogFlags(&Result));
	return	Result;
}
size_t		NetQuota::Impl::GetDefaultLimit() const {
	LONGLONG	Result;
	err(E_FAIL);
	if (pDQControl)
		err(pDQControl->GetDefaultQuotaLimit(&Result));
	return	SUCCEEDED(err()) ? Bytes2Mega(Result) : 0;
}
size_t		NetQuota::Impl::GetDefaultThreshold() const {
	LONGLONG	Result;
	err(E_FAIL);
	if (pDQControl)
		err(pDQControl->GetDefaultQuotaThreshold(&Result));
	return	SUCCEEDED(err()) ? Bytes2Mega(Result) : 0;
}
AutoUTF		NetQuota::Impl::GetDefaultLimitText() const {
	AutoUTF	Result;
	err(E_FAIL);
	if (pDQControl) {
		WinBuf<WCHAR>	buf(MAX_PATH);
		err(pDQControl->GetDefaultQuotaLimitText(buf, buf.capacity()));
		Result = buf.data();
	}
	return	Result;
}
AutoUTF		NetQuota::Impl::GetDefaultThresholdText() const {
	AutoUTF	Result;
	err(E_FAIL);
	if (pDQControl) {
		WinBuf<WCHAR>	buf(MAX_PATH);
		err(pDQControl->GetDefaultQuotaThresholdText(buf, buf.capacity()));
		Result = buf.data();
	}
	return	Result;
}

AutoUTF		NetQuota::Impl::ParseState(DWORD in) {
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_ENFORCE))
		return	L"Quotas are enabled and the limit value is enforced. Users cannot exceed their quota limit.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_TRACK))
		return	L"Quotas are enabled but the limit value is not being enforced. Users may exceed their quota limit.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_REBUILDING))
		return	L"The volume is rebuilding its quota information.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_FILESTATE_INCOMPLETE))
		return	L"The volume's quota information is out of date. Quotas are probably disabled.";
	if (WinFlag::Check(in, (DWORD)DISKQUOTA_STATE_DISABLED))
		return	L"Quotas are not enabled on the volume.";
	return	L"Unknown State";
}
bool		NetQuota::Impl::IsSupport(const AutoUTF &path) {
	DWORD	dwFlags = 0;
	::GetVolumeInformationW(path.c_str(), null_ptr, 0, null_ptr, null_ptr, &dwFlags, null_ptr, 0);
	return	(WinFlag::Check(dwFlags, (DWORD)FILE_VOLUME_QUOTAS));
}

/*
bool		NetQuota::Add(const AutoUTF &name, LONGLONG lim, LONGLONG thr) {
	err = E_FAIL;
	if (pDQControl) {
		PDISKQUOTA_USER usr = null_ptr;
		Sid sid(name, L"");
		err = pDQControl->FindUserSid(sid, DISKQUOTA_USERNAME_RESOLVE_SYNC, &usr);
		if (SUCCEEDED(err) && usr) {
			usr->SetQuotaLimit(Mega2Bytes(lim), true);
			usr->SetQuotaThreshold(Mega2Bytes(thr), true);
			usr->Release();
		}
	}
	return	SUCCEEDED(err);
}
HRESULT		NetQuota::UserFind(const AutoUTF &name) {
	HRESULT err = E_FAIL;
	if (pDQControl) {
		Sid sid(name, L"");
//		hr = pDQControl->FindUserSid(sid.GetSid(), DISKQUOTA_USERNAME_RESOLVE_ASYNC, &pDQUser);
//		if (FAILED(hr)) pDQUser = null_ptr;
	}
	return	err;
}
*/

///======================================================================================== NetQuota
NetQuota::NetQuota(const AutoUTF &path): pimpl(new Impl(path)) {
}
PVOID		NetQuota::GetDQC() const {
	return	pimpl->GetDQC();
}

//change state
bool		NetQuota::SetState(QuotaState in) const {
	return	pimpl->SetState(in);
}
bool		NetQuota::Disable() const {
	return	pimpl->SetState(stDisable);
}
bool		NetQuota::Enable() const {
	return	pimpl->SetState(stEnable);
}
bool		NetQuota::Track() const {
	return	pimpl->SetState(stTrack);
}

//check state
bool		NetQuota::IsStateDisabled() const {
	return	pimpl->IsStateDisabled();
}
bool		NetQuota::IsStateTracked() const {
	return	pimpl->IsStateTracked();
}
bool		NetQuota::IsStateEnforced() const {
	return	pimpl->IsStateEnforced();
}

//change limits
bool		NetQuota::SetLogLimit(bool in) const {
	return	pimpl->SetLogLimit(in);
}
bool		NetQuota::SetLogTreshold(bool in) const {
	return	pimpl->SetLogTreshold(in);
}
bool		NetQuota::SetDefaultLimit(size_t in) const {
	return	pimpl->SetDefaultLimit(in);
}
bool		NetQuota::SetDefaultThreshold(size_t in) const {
	return	pimpl->SetDefaultThreshold(in);
}

AutoUTF		NetQuota::path() const {
	return	pimpl->path();
}
DWORD		NetQuota::GetState() const {
	return	pimpl->GetState();
}
DWORD		NetQuota::GetLogFlags() const {
	return	pimpl->GetLogFlags();
}
size_t		NetQuota::GetDefaultLimit() const {
	return	pimpl->GetDefaultLimit();
}
size_t		NetQuota::GetDefaultThreshold() const {
	return	pimpl->GetDefaultThreshold();
}
AutoUTF		NetQuota::GetDefaultLimitText() const {
	return	pimpl->GetDefaultLimitText();
}
AutoUTF		NetQuota::GetDefaultThresholdText() const {
	return	pimpl->GetDefaultThresholdText();
}

//	AutoUTF			ParseState() const {
//		return	ParseState(GetState());
//	}
//	static AutoUTF	ParseState(DWORD in);

bool		NetQuota::IsSupport(const AutoUTF &path) {
	return	Impl::IsSupport(path);
}

///============================================================================= NetQuotaUsers::Impl
class		NetQuotaUsers::Impl : public MapContainer<AutoUTF, QuotaInfo> {
	IDiskQuotaControl	*pDQControl;
	IEnumDiskQuotaUsers	*pEnumDQUsers;

	HRESULT			EnumStartCache();
	HRESULT			EnumOpen();
	void			EnumClose();

public:
	~Impl() {
		EnumClose();
	}
	Impl(const NetQuota &in, bool autocache = true);

	bool			Cache();

	HRESULT			Add(const AutoUTF &name, LONGLONG lim = 0, LONGLONG thr = 0);
	HRESULT			Del(const AutoUTF &name);
	HRESULT			Del();

	AutoUTF			GetName() const;
	size_t 			GetLimit() const;
	size_t			GetThreshold() const;
	size_t			GetUsed() const;

	HRESULT			SetLimit(LONGLONG in);
	HRESULT			SetThreshold(LONGLONG in);
};

//private
HRESULT		NetQuotaUsers::Impl::EnumStartCache() {
	EnumClose();
	HRESULT	err = pDQControl->CreateEnumUsers(null_ptr, 0, DISKQUOTA_USERNAME_RESOLVE_ASYNC, &pEnumDQUsers);
	return	err;
}
HRESULT		NetQuotaUsers::Impl::EnumOpen() {
	EnumClose();
	HRESULT	err = pDQControl->CreateEnumUsers(null_ptr, 0, DISKQUOTA_USERNAME_RESOLVE_NONE, &pEnumDQUsers);
	if (SUCCEEDED(err))
		Clear();
	return	err;
}
void		NetQuotaUsers::Impl::EnumClose() {
	if (pEnumDQUsers) {
		pEnumDQUsers->Release();
		pEnumDQUsers = null_ptr;
	}
}

//public
NetQuotaUsers::Impl::Impl(const NetQuota &in, bool autocache): pDQControl((IDiskQuotaControl*)in.GetDQC()), pEnumDQUsers(null_ptr) {
	if (autocache)
		Cache();
}

bool		NetQuotaUsers::Impl::Cache() {
	bool	Result = false;
	IDiskQuotaUser		*pDQUser		= null_ptr;
	if (pDQControl) {
		EnumStartCache();
		if (SUCCEEDED(EnumOpen())) {
			while (pEnumDQUsers->Next(1, &(pDQUser), null_ptr) == S_OK) {
				QuotaInfo	info;
				LONGLONG	tmp;
				pDQUser->GetQuotaLimit(&tmp);
				info.usr	= pDQUser;
				info.limit	= Bytes2Mega(tmp);
				pDQUser->GetQuotaThreshold(&tmp);
				info.thres	= Bytes2Mega(tmp);
				DWORD	sidlen;
				pDQUser->GetSidLength(&sidlen);
				PSID	sidbuf;
				WinMem::Alloc(sidbuf, sidlen);
				pDQUser->GetSid((PBYTE)sidbuf, sidlen);
				AutoUTF name = Sid::AsName(sidbuf);
				if (!name.empty())
					Insert(name, info);
				WinMem::Free(sidbuf);
			}
			Result = true;
		}
	}
	return	Result;
}

HRESULT		NetQuotaUsers::Impl::Add(const AutoUTF &name, LONGLONG lim, LONGLONG thr) {
	HRESULT err = E_FAIL;
	if (!Find(name)) {
		if (pDQControl) {
			PDISKQUOTA_USER usr = null_ptr;
			err = pDQControl->AddUserName(name.c_str(), DISKQUOTA_USERNAME_RESOLVE_SYNC, &usr);
			if (SUCCEEDED(err) && usr) {
				usr->SetQuotaLimit(Mega2Bytes(lim), true);
				usr->SetQuotaThreshold(Mega2Bytes(thr), true);
				QuotaInfo info;
				info.usr	= usr;
				info.limit	= lim;
				info.thres	= thr;
				Insert(name, info);
			}
		}
	} else {
		SetLimit(lim);
		SetThreshold(thr);
	}
	return	err;
}
HRESULT		NetQuotaUsers::Impl::Del(const AutoUTF &name) {
	return	(Find(name)) ? Del() : E_FAIL;
}
HRESULT		NetQuotaUsers::Impl::Del() {
	HRESULT hr = E_FAIL;
	if (ValidPtr()) {
		hr = pDQControl->DeleteUser(Value().usr);
		if (SUCCEEDED(hr)) {
			(Value().usr)->Release();
			Erase();
		}
	}
	return	(hr);
}

AutoUTF		NetQuotaUsers::Impl::GetName() const {
	return	(ValidPtr()) ? Key() : L"";
}
size_t		NetQuotaUsers::Impl::GetLimit() const {
	return	(ValidPtr()) ? Value().limit : 0;
}
size_t		NetQuotaUsers::Impl::GetThreshold() const {
	return	(ValidPtr()) ? Value().thres : 0;
}
size_t		NetQuotaUsers::Impl::GetUsed() const {
	DWORD	Result = 0;
	if (ValidPtr()) {
		LONGLONG	tmp;
		Value().usr->GetQuotaUsed(&tmp);
		Result = Bytes2Mega(tmp);
	}
	return	Result;
}

HRESULT		NetQuotaUsers::Impl::SetLimit(LONGLONG in) {
	HRESULT err = E_FAIL;
	if (ValidPtr() && Value().usr)
		err = Value().usr->SetQuotaLimit(Mega2Bytes(in), true);
	if (SUCCEEDED(err))
		Value().limit = in;
	return	err;
}
HRESULT		NetQuotaUsers::Impl::SetThreshold(LONGLONG in) {
	HRESULT err = E_FAIL;
	if (ValidPtr() && Value().usr)
		err = Value().usr->SetQuotaThreshold(Mega2Bytes(in), true);
	if (SUCCEEDED(err))
		Value().thres = in;
	return	err;
}

///=================================================================================== NetQuotaUsers
NetQuotaUsers::NetQuotaUsers(const NetQuota &in, bool autocache): pimpl(new Impl(in, autocache)) {
}

bool		NetQuotaUsers::Cache() {
	return	pimpl->Cache();
}

HRESULT		NetQuotaUsers::Add(const AutoUTF &name, LONGLONG lim, LONGLONG thr) {
	return	pimpl->Add(name, lim, thr);
}
HRESULT		NetQuotaUsers::Del(const AutoUTF &name) {
	return	pimpl->Del(name);
}
HRESULT		NetQuotaUsers::Del() {
	return	pimpl->Del();
}

AutoUTF		NetQuotaUsers::GetName() const {
	return	pimpl->GetName();
}
size_t 		NetQuotaUsers::GetLimit() const {
	return	pimpl->GetLimit();
}
size_t		NetQuotaUsers::GetThreshold() const {
	return	pimpl->GetThreshold();
}
size_t		NetQuotaUsers::GetUsed() const {
	return	pimpl->GetUsed();
}

HRESULT		NetQuotaUsers::SetLimit(LONGLONG in) {
	return	pimpl->SetLimit(in);
}
HRESULT		NetQuotaUsers::SetThreshold(LONGLONG in) {
	return	pimpl->SetThreshold(in);
}

bool		NetQuotaUsers::Find(const AutoUTF &name) const {
	return	pimpl->Find(name);
}
size_t		NetQuotaUsers::Size() const {
	return	pimpl->Size();
}
