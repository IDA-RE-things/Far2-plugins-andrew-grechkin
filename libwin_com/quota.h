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

#include <iosfwd>

////▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ com_quota
///======================================================================================== NetQuota
struct IDiskQuotaControl;
struct IDiskQuotaUser;
struct IEnumDiskQuotaUsers;

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

	ComObject<IDiskQuotaUser> Add(const AutoUTF &name, size_t lim = 0, size_t thr = 0);

	void Del(const AutoUTF &name);

	static bool IsSupported(const AutoUTF &path);

private:
	ComObject<IDiskQuotaUser> add_user(const AutoUTF &name) const;
	ComObject<IDiskQuotaUser> get_user(const AutoUTF &name) const;

	AutoUTF m_path;
	ComObject<IDiskQuotaControl> m_control;
};

///=================================================================================== NetQuotaUsers
struct QuotaInfo {
public:
	QuotaInfo(const ComObject<IDiskQuotaControl> &c, PCWSTR name);
	QuotaInfo(const ComObject<IDiskQuotaUser> &u);

	AutoUTF name() const;
	AutoUTF used_text() const;
	AutoUTF limit_text() const;
	AutoUTF threshold_text() const;

	size_t used() const;
	size_t limit() const;
	size_t threshold() const;

	void set_limit(size_t in);
	void set_threshold(size_t in);

private:
	ComObject<IDiskQuotaUser> m_usr;
	AutoUTF m_name;
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
	NetQuota m_nq;
};

#endif
