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
///======================================================================================== NetQuota
struct IDiskQuotaControl;
struct NetQuota {
	~NetQuota();

	NetQuota(const AutoUTF &path);

	const ComObject<IDiskQuotaControl>& operator->() const;

	AutoUTF path() const;

	//change state
	void SetState(DWORD in) const;

	void Disable() const;

	void Enable() const;

	void Track() const;

	//check state
	bool IsDisabled() const;

	bool IsEnabled() const;

	bool IsTracked() const;

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
	AutoUTF m_path;
	ComObject<IDiskQuotaControl> pDQControl;
};

///=================================================================================== NetQuotaUsers
struct IDiskQuotaUser;
struct IEnumDiskQuotaUsers;
struct QuotaInfo {
	ComObject<IDiskQuotaUser> usr;
	size_t used;
	size_t limit;
	size_t thres;
	QuotaInfo(const ComObject<IDiskQuotaUser> &u);
	QuotaInfo(const ComObject<IDiskQuotaUser> &u, size_t s, size_t l, size_t t);
};

struct NetQuotaUsers: public MapContainer<AutoUTF, QuotaInfo> {
	~NetQuotaUsers();

	NetQuotaUsers(const NetQuota &nq);

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
	const NetQuota &m_nq;
};

#endif
