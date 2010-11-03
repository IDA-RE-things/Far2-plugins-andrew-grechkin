/**
 * win_com
 * @classes	()
 * @author	2010 Andrew Grechkin
 * @link	(ole32)
 **/

#ifndef WIN_COM_HPP
#define WIN_COM_HPP

#include "win_net.h"

// нельзя убирать в cpp, ругается линкер
#include <mstask.h>

///========================================================================================== WinCom
/// Класс инициализации COM singletone (объекты создавать запрещено, нужно использовать фукцию init)
struct WinCOM {
	~WinCOM();

	static WinCOM &init();

private:
	WinCOM(const WinCOM&);

	WinCOM();
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ com_quota
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

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ com_task
///====================================================================================== WinTrigger
struct WinTrigger {
	WinTrigger();

	WinTrigger(PCWSTR str);

	WinTrigger(const AutoUTF &period, const AutoUTF &tag1 = L"", const AutoUTF &tag2 = L"");

	TASK_TRIGGER* Info() const {
		return (TASK_TRIGGER*)m_info.get();
	}

	void SetTime(DWORD h = 0, DWORD m = 0);

	void Period(const AutoUTF &period, const AutoUTF &tag1 = L"", const AutoUTF &tag2 = L"");

	void RepeatMin(DWORD every = 60, DWORD forwhile = 24 * 60);

	void RepeatWeek(const AutoUTF &interval, const AutoUTF &in);

	void RepeatMonth(const AutoUTF &in, const AutoUTF &day);

	DWORD GetStartHour() const;

	DWORD GetStartMinute() const;

	DWORD GetType() const;

	DWORD GetDailyInterval() const;

	DWORD GetWeeklyDow() const;

	DWORD GetWeeklyInterval() const;

	DWORD GetMonthlyMonth() const;

	DWORD GetMonthlyDom() const;

	DWORD GetRepeatHour() const;

	DWORD GetRepeatMin() const;

	DWORD GetRepeatDuraHour() const;

	DWORD GetRepeatDuraMin() const;

	bool IsRepeat() const;

	AutoUTF GetPeriod() const;

	AutoUTF AsStr() const;

private:
	WinTrigger(const WinTrigger &);
	WinTrigger& operator=(const WinTrigger &);
	void Init();

	typedef shared_ptr<TASK_TRIGGER> info_ptr;
	info_ptr m_info;
	AutoUTF m_str;
};

///========================================================================================= WinTask
struct WinTask {
	ITask* Info() const {
		return m_pTask;
	}

	ITask* operator->() const {
		return m_pTask;
	}

	~WinTask();

	WinTask(const AutoUTF &name, bool autocreate = true);

	void Run() const;

	void Disable();

	void Enable();

	void SetFlags(DWORD in, bool on = true);

	void SetAccount(const AutoUTF &name, PCWSTR pass = nullptr);

	void SetApplication(const AutoUTF &path);

	void SetCreator(const AutoUTF &name);

	void SetParam(const AutoUTF &in);

	void SetWorkdir(const AutoUTF &path);

	void SetComment(const AutoUTF &in);

	void SetMaxRunTime(DWORD in);

	void AddTrigger(const WinTrigger &in);

	void SetTrigger(DWORD index, const WinTrigger &in);

	void CleanTriggers();

	void Save() const;

	DWORD GetFlags() const;

	AutoUTF GetAccount() const;

	AutoUTF GetApplication() const;

	AutoUTF GetCreator() const;

	AutoUTF GetParam() const;

	AutoUTF GetWorkdir() const;

	AutoUTF GetComment() const;

	static bool IsExist(const AutoUTF &name);

	static bool IsDisabled(const AutoUTF &name);

	static bool IsRunning(const AutoUTF &name);

	static void Del(const AutoUTF &name);

	static DWORD GetFlags(const AutoUTF &name);

	static HRESULT GetState(const AutoUTF &name);

	static AutoUTF ParseState(HRESULT in);

	static void ParseCommand(const AutoUTF &comm, AutoUTF &app, AutoUTF &par);

private:
	ITask *m_pTask;
};

///===================================================================================== WinTriggers
class WinTriggers: public MapContainer<DWORD, shared_ptr<WinTrigger> > {
	const WinTask &m_task;

	WinTriggers(const WinTriggers&);

	WinTriggers& operator=(const WinTriggers&);

public:
	~WinTriggers();

	WinTriggers(const WinTask &task, bool autocache = true);

	void Cache();

	WinTrigger& Get();

	DWORD GetStartHour() const;

	DWORD GetStartMinute() const;

	DWORD GetType() const;

	AutoUTF GetDailyInterval() const;

	AutoUTF GetWeeklyDow() const;

	AutoUTF GetWeeklyInterval() const;

	AutoUTF GetMonthlyMonth() const;

	AutoUTF GetMonthlyDom() const;

	AutoUTF GetRepeatHour() const;

	AutoUTF GetRepeatMin() const;

	AutoUTF GetRepeatDuraHour() const;

	AutoUTF GetRepeatDuraMin() const;

	bool IsRepeat() const;

	AutoUTF AsStr() const;

	AutoUTF AsStrAll(const AutoUTF &delim = L", ");
};

///============================================================================================ Cron
struct TaskInfo {
	AutoUTF app;
	AutoUTF par;
	AutoUTF path;
	AutoUTF comm;
//	AutoUTF acc;
	AutoUTF creator;
	DWORD flags;

	TaskInfo();

	TaskInfo(const WinTask &task);
};

class Cron: public MapContainer<AutoUTF, TaskInfo> {
	AutoUTF m_creator;

public:
	Cron(bool autocache = true) :
			m_creator(L"") {
		if (autocache)
			Cache();
	}

	void Cache() {
		CacheByCreator();
	}

	void CacheByCreator(const AutoUTF &in = L"");

	void Add(const AutoUTF &name);

	void Del();

	void Disable();

	void Enable();

	void SetApp(const AutoUTF &in);

	void SetCreator(const AutoUTF &in);

	void SetParam(const AutoUTF &in);

	void SetPath(const AutoUTF &in);

	void SetComm(const AutoUTF &in);

	void SetAcc(const AutoUTF &name, PCWSTR pass);

	void TriggerAdd(const WinTrigger &in) const;

	void TriggerClean() const;

	AutoUTF GetName() const;

	AutoUTF GetApp() const;

	AutoUTF GetCreator() const;

	AutoUTF GetParam() const;

	AutoUTF GetPath() const;

	AutoUTF GetComm() const;

	AutoUTF GetTriggersString(const AutoUTF &delim = L", ") const;

	DWORD GetFlags() const;
};

#endif // WIN_COM_HPP
