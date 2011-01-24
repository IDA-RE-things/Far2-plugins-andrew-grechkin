/**
 * quota
 * @classes		()
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#ifndef WIN_COM_QUOTA_HPP
#define WIN_COM_QUOTA_HPP

#include <libwin_net/win_net.h>
#include "win_com.h"

////▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ com_quota
enum QuotaState {
	stDisable, stEnable, stTrack,
};

///======================================================================================== NetQuota
struct IDiskQuotaControl;
struct NetQuota {
	~NetQuota();

	NetQuota(const AutoUTF &path);

	IDiskQuotaControl* Info() const;

	AutoUTF path() const;

	//change state
	void SetState(QuotaState in) const;

	void Disable() const;

	void Enable() const;

	void Track() const;

	//check state
	bool IsStateDisabled() const;

	bool IsStateTracked() const;

	bool IsStateEnforced() const;

	bool IsLogLimit() const;

	bool IsLogThreshold() const;

	//change limits
	void SetDefaultLimit(size_t in = 0) const;

	void SetDefaultThreshold(size_t in = 0) const;

	void SetLogLimit(bool in = true) const;

	void SetLogThreshold(bool in = true) const;

	DWORD GetState() const;

	DWORD GetLogFlags() const;

	size_t GetDefaultLimit() const;

	size_t GetDefaultThreshold() const;

	AutoUTF GetDefaultLimitText() const;

	AutoUTF GetDefaultThresholdText() const;

	AutoUTF ParseState() const;

	static bool IsSupportQuota(const AutoUTF &path);

private:
	IDiskQuotaControl *pDQControl;
	AutoUTF m_path;
};

///=================================================================================== NetQuotaUsers
struct IDiskQuotaUser;
struct IEnumDiskQuotaUsers;
struct QuotaInfo {
	IDiskQuotaUser *usr;
	size_t used;
	size_t limit;
	size_t thres;
	QuotaInfo(IDiskQuotaUser *u, size_t s, size_t l, size_t t) :
			usr(u), used(s), limit(l), thres(t) {
	}
};

struct NetQuotaUsers: public MapContainer<AutoUTF, QuotaInfo> {
	~NetQuotaUsers();

	NetQuotaUsers(const NetQuota &nq, bool autocache = true);

	void Cache();

	void Add(const AutoUTF &name, size_t lim = 0, size_t thr = 0);

	void Del(const AutoUTF &name);

	void Del();

	AutoUTF GetName() const;

	AutoUTF GetUsedText() const;

	AutoUTF GetLimitText() const;

	AutoUTF GetThresholdText() const;

	size_t GetUsed() const;

	size_t GetLimit() const;

	size_t GetThreshold() const;

	void SetLimit(size_t in);

	void SetThreshold(size_t in);

private:
	void PreCache();

	void EnumClose();

	void EnumOpen();

	const NetQuota &m_nq;
	IDiskQuotaControl *pDQControl;
	IEnumDiskQuotaUsers *pEnumDQUsers;
};

#endif
