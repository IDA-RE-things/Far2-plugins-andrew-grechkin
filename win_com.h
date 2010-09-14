/**
	win_com

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_COM_HPP
#define WIN_COM_HPP

#include "win_def.h"
//#include "win_net.h"

#include "win_c_map.h"
#include "win_container.h"

#include <mstask.h>
#include <time.h>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_quota
#include <memory>
using std::auto_ptr;

enum		QuotaState {stDisable, stEnable, stTrack};

///======================================================================================== NetQuota
class		NetQuota {
	class		Impl;
	typedef		auto_ptr<Impl>	impl_ptr;
	impl_ptr	pimpl;
public:
	NetQuota(const AutoUTF &path);
	PVOID			GetDQC() const;

	//change state
	bool			SetState(QuotaState in) const;
	bool			Disable() const;
	bool			Enable() const;
	bool			Track() const;

	//check state
	bool			IsStateDisabled() const;
	bool			IsStateTracked() const;
	bool			IsStateEnforced() const;

	//change limits
	bool			SetLogLimit(bool in = true) const;
	bool			SetLogTreshold(bool in = true) const;
	bool			SetDefaultLimit(size_t in = 0) const;
	bool			SetDefaultThreshold(size_t in = 0) const;

	AutoUTF			path() const;
	DWORD			GetState() const;
	DWORD			GetLogFlags() const;
	size_t			GetDefaultLimit() const;
	size_t			GetDefaultThreshold() const;
	AutoUTF			GetDefaultLimitText() const;
	AutoUTF			GetDefaultThresholdText() const;

//	AutoUTF			ParseState() const {
//		return	ParseState(GetState());
//	}
//	static AutoUTF	ParseState(DWORD in);

	static bool		IsSupport(const AutoUTF &path);
};

///=================================================================================== NetQuotaUsers
class		NetQuotaUsers {
	class		Impl;
	typedef		auto_ptr<Impl>	impl_ptr;
	impl_ptr	pimpl;
public:
	NetQuotaUsers(const NetQuota &in, bool autocache = true);

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

	bool			Find(const AutoUTF &name) const;
	size_t			Size() const;
};

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_task
struct		TriggerInfo : public TASK_TRIGGER {
	TriggerInfo() {
		Init();
	}
	TriggerInfo(const AutoUTF &in, const AutoUTF &tag1 = L"", const AutoUTF &tag2 = L"") {
		Init();
		Period(in, tag1, tag2);
	}
	void	Init() {
		ZeroMemory(this, sizeof(TriggerInfo));
		cbTriggerSize	= sizeof(TriggerInfo);
		time_t tTime	= time(NULL);
		tm *lt			= localtime(&tTime);
		wBeginDay		= lt->tm_mday;
		wBeginMonth		= lt->tm_mon + 1;
		wBeginYear		= lt->tm_year + 1900;
	}
	void	Time(DWORD h = 0, DWORD m = 0) {
		wStartHour		= h;
		wStartMinute	= m;
	}
	void	Period(const AutoUTF &in, const AutoUTF &tag1 = L"", const AutoUTF &tag2 = L"") {
		if (in == L"daily") {
			TriggerType = TASK_TIME_TRIGGER_DAILY;
			Type.Daily.DaysInterval = (tag1.empty()) ? 1 : AsUInt(tag1.c_str());
		}
		if (in == L"weekly") {
			RepeatWeek(tag1, tag2);
		}
		if (in == L"monthly") {
			RepeatMonth(tag1, tag2);
		}
		if (in == L"reboot") {
			TriggerType = TASK_EVENT_TRIGGER_AT_SYSTEMSTART;
		}
	}
	void	RepeatMin(DWORD every = 60, DWORD forwhile = 24*60) {
		MinutesInterval = every;
		MinutesDuration = forwhile;
	}
	void	RepeatWeek(const AutoUTF &interval, const AutoUTF &in) {
		if (interval.empty() || in.empty())
			return;
		DWORD	inter	= AsUInt(interval.c_str());
		DWORD	dow		= BitMask<DWORD>::FromStr(in, 7);
		if (WinBit::Check(dow, 6)) {
			WinBit::UnSet(dow, 6);
			dow = dow << 1;
			WinBit::Set(dow, 0);
		} else {
			dow = dow << 1;
		}

		if ((dow == 0) || (inter == 0))
			return;
		TriggerType = TASK_TIME_TRIGGER_WEEKLY;
		Type.Weekly.WeeksInterval = inter;
		Type.Weekly.rgfDaysOfTheWeek = dow;
	}
	void	RepeatMonth(const AutoUTF &in, const AutoUTF &day) {
		if (day.empty() || in.empty())
			return;
		DWORD	mon	= BitMask<DWORD>::FromStr(in, 12);
		DWORD	dom	= BitMask<DWORD>::FromStr(day, 31);
		if ((mon == 0) || (dom == 0))
			return;
		TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;
		Type.MonthlyDate.rgfMonths = mon;
		Type.MonthlyDate.rgfDays = dom;
	}
};

struct		TaskInfo {
	AutoUTF		app;
	AutoUTF		par;
	AutoUTF		path;
	AutoUTF		comm;
	AutoUTF		acc;
	AutoUTF		creat;
};

///==================================================================================== WinScheduler
class		WinScheduler {
	ITaskScheduler	*pITS;

	WinScheduler(): pITS(NULL) {
		HRESULT	hr = ::CoCreateInstance(CLSID_CTaskScheduler,
										NULL,
										CLSCTX_INPROC_SERVER,
										IID_ITaskScheduler,
										(PVOID*) & pITS);
		if (FAILED(hr)) {
			pITS = NULL;
		}
	}
	WinScheduler(const WinScheduler&);	// deny copy constructor
public:
	~WinScheduler() {
		if (pITS) {
			pITS->Release();
		}
	}

	operator				ITaskScheduler*() {
		return	pITS;
	}

	static WinScheduler&	the() { // only one instance allowed
		static WinScheduler	sche;
		return	sche;
	}
	static bool				IsOK() {
		return	the().pITS != NULL;
	}
	static ITask*			GetTask(PCWSTR task) {
		ITaskScheduler	*pTS = WinScheduler::the();
		if (!IsOK())
			return	NULL;
		ITask			*pTask;
		HRESULT hr = pTS->Activate(task, IID_ITask, (IUnknown**) & pTask);
		if (FAILED(hr))
			pTask = NULL;
		return	pTask;
	}
};

///===================================================================================== WinTriggers
class		WinTriggers : public MapContainer<DWORD, TriggerInfo> {
	AutoUTF		_task;
	WinTriggers(const WinTriggers&);
public:
	~WinTriggers() {}
	WinTriggers(const AutoUTF &task, bool autocache = true);
	bool					Cache(const AutoUTF &task = L"");
	TriggerInfo*			GetTrigger() {
		return	(ValidPtr()) ? &(Value()) : NULL;
	}
	bool					Add(const TriggerInfo &in);
	bool					Set(const TriggerInfo &in);
	bool					Get(TriggerInfo &in);
	bool					Clean();
	AutoUTF					GetPeriod();
	DWORD					GetStartHour() {
		return	(ValidPtr()) ? Value().wStartHour : 0;
	}
	DWORD					GetStartMinute() {
		return	(ValidPtr()) ? Value().wStartMinute : 0;
	}
	DWORD					GetType() {
		return	(ValidPtr()) ? Value().TriggerType : 0;
	}
	AutoUTF					GetDailyInterval() {
		return	Num2Str(Value().Type.Daily.DaysInterval);
	}
	AutoUTF					GetWeeklyDow() {
		DWORD tmp = Value().Type.Weekly.rgfDaysOfTheWeek;
		if (WinBit::Check(tmp, 0)) {
			tmp = tmp >> 1;
			WinBit::Set(tmp, 6);
		} else {
			tmp = tmp >> 1;
		}
		return	BitMask<DWORD>::AsStr(tmp, 7);
	}
	AutoUTF					GetWeeklyInterval() {
		return	Num2Str(Value().Type.Weekly.WeeksInterval);
	}
	AutoUTF					GetMonthlyMonth() {
		return	BitMask<DWORD>::AsStr(Value().Type.MonthlyDate.rgfMonths, 12);
	}
	AutoUTF					GetMonthlyDom() {
		return	BitMask<DWORD>::AsStr(Value().Type.MonthlyDate.rgfDays, 31);
	}
	AutoUTF					GetRepeatHour() {
		return	Num2Str(Value().MinutesInterval / 60);
	}
	AutoUTF					GetRepeatMin() {
		return	Num2Str(Value().MinutesInterval - (Value().MinutesInterval / 60)*60);
	}
	AutoUTF					GetRepeatDuraHour() {
		return	Num2Str(Value().MinutesDuration / 60);
	}
	AutoUTF					GetRepeatDuraMin() {
		return	Num2Str(Value().MinutesDuration - (Value().MinutesDuration / 60)*60);
	}
	bool					IsRepeat() {
		return	((Value().MinutesInterval != 0) && (Value().MinutesDuration != 0));
	}
};

///=============================================================================== WinTriggersString
class		WinTriggersString : public ListContainer<AutoUTF> {
	WinTriggersString(const WinTriggersString&);
public:
	~WinTriggersString() {}
	WinTriggersString(const AutoUTF &task);
	AutoUTF					GetString() {
		return	(ValidPtr()) ? Value() : L"";
	}
	AutoUTF					GetAll(const AutoUTF &in = L", ");
};

///========================================================================================= WinTask
class		WinTask {
	ITaskScheduler* pTS;
	WinTask(const WinTask&);			// deny copy constructor

public:
	void 					CreateTrigger(const AutoUTF &task);
	void 					LoadTrigger(const AutoUTF &task, WinTriggers &nt);

	static bool				IsExist(const AutoUTF &task);
	static bool				IsDisabled(const AutoUTF &task) {
		return	(GetFlags(task) & TASK_FLAG_DISABLED) ? true : false;
	}
	static bool				IsRunning(const AutoUTF &task) {
		return	(GetState(task) == SCHED_S_TASK_RUNNING) ? true : false;
	}
	static DWORD			GetFlags(const AutoUTF &task);
	static HRESULT			GetState(const AutoUTF &task);

	static void				StopChanging(ITask *pTask);
	static bool				Add(const AutoUTF &task);
	static bool				Del(const AutoUTF &task);
	static bool				SetAcc(const AutoUTF &task, const AutoUTF &name, PCWSTR pass);
	static bool				SetApp(const AutoUTF &task, const AutoUTF &in);
	static bool				SetCreator(const AutoUTF &task, const AutoUTF &in);
	static bool				SetPar(const AutoUTF &task, const AutoUTF &in);
	static bool				SetPath(const AutoUTF &task, const AutoUTF &in);
	static void				SetComm(const AutoUTF &task, const AutoUTF &in);
	static void				SetMaxRunTime(const AutoUTF &task, DWORD in);
	static void				SetFlags(const AutoUTF &task, DWORD in, bool on = true);
	static void				UnSetFlags(const AutoUTF &task, DWORD in);
	static bool				Run(const AutoUTF &task);

	static void				Disable(const AutoUTF &task);
	static void				Enable(const AutoUTF &task);
	static AutoUTF			GetAcc(const AutoUTF &task);
	static AutoUTF			GetApp(const AutoUTF &task);
	static AutoUTF			GetCreator(const AutoUTF &task);
	static AutoUTF			GetPar(const AutoUTF &task);
	static AutoUTF			GetPath(const AutoUTF &task);
	static AutoUTF			GetComm(const AutoUTF &task);
	static AutoUTF			ParseState(HRESULT in);
	static void				ParseCommand1(const AutoUTF &comm, AutoUTF &app, AutoUTF &par);
};

///============================================================================================ Cron
class		Cron : public MapContainer<AutoUTF, TaskInfo> {
	AutoUTF		_creator;
public:
	Cron(bool autocache = true): _creator(L"") {
		if (autocache)
			Cache();
	}
	bool					Cache() {
		return	CacheByCreator();
	}
	bool					CacheByCreator(const AutoUTF &in = L"");
	bool					Add(const AutoUTF &name);
	bool					Del();
	bool					Disable();
	bool					Enable();
	void					SetApp(const AutoUTF &in);
	void					SetCreator(const AutoUTF &in);
	void					SetPar(const AutoUTF &in);
	void					SetPath(const AutoUTF &in);
	void					SetComm(const AutoUTF &in);
	void					SetAcc(const AutoUTF &name, PCWSTR pass);
	void					TriggerAdd(const TriggerInfo &in) const;
	void					TriggerClean() const;
	AutoUTF					GetName() const;
	AutoUTF					GetApp() const;
	AutoUTF					GetCreator() const;
	AutoUTF					GetPar() const;
	AutoUTF					GetPath() const;
	AutoUTF					GetComm() const;
	AutoUTF					GetTriggersString(const AutoUTF &in = L", ") const;
	DWORD					GetFlags() const;
};

#endif // WIN_COM_HPP
