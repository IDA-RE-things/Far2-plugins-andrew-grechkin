/**
 * task
 * manipulate Task Scheduler ver 1
 * @classes		(WinScheduler, WinTriggers, WinTask, Cron)
 * @author		Copyright © 2011 Andrew Grechkin
 * @link		(ole32, oleaut32, uuid)
**/

#include "task.h"

#include <time.h>

///==================================================================================== WinScheduler
struct WinScheduler {
	~WinScheduler() {
		m_pTS->Release();
	}

	ITaskScheduler* operator->() {
		return m_pTS;
	}

	static WinScheduler& instance() { // only one instance allowed
		WinCOM::init();
		static WinScheduler sche;
		return sche;
	}

private:
	ITaskScheduler *m_pTS;

	WinScheduler() :
			m_pTS(nullptr) {
		CheckApiError(::CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER,
										 IID_ITaskScheduler, (PVOID*)&m_pTS));
	}

	WinScheduler(const WinScheduler&); // deny copy constructor
	WinScheduler& operator=(const WinScheduler&); // deny operator =
};

///====================================================================================== WinTrigger
WinTrigger::WinTrigger() :
		m_info(new _TASK_TRIGGER) {
	Init();
}

WinTrigger::WinTrigger(PCWSTR str) :
		m_info(new _TASK_TRIGGER), m_str(str) {
}

WinTrigger::WinTrigger(const AutoUTF &period, const AutoUTF &tag1, const AutoUTF &tag2) :
		m_info(new _TASK_TRIGGER) {
	Init();
	Period(period, tag1, tag2);
}

void WinTrigger::SetTime(DWORD h, DWORD m) {
	m_info->wStartHour = h;
	m_info->wStartMinute = m;
}

void WinTrigger::Period(const AutoUTF &period, const AutoUTF &tag1, const AutoUTF &tag2) {
	if (period == L"daily") {
		m_info->TriggerType = TASK_TIME_TRIGGER_DAILY;
		m_info->Type.Daily.DaysInterval = (tag1.empty()) ? 1 : AsUInt(tag1.c_str());
	} else if (period == L"weekly") {
		RepeatWeek(tag1, tag2);
	} else if (period == L"monthly") {
		RepeatMonth(tag1, tag2);
	} else if (period == L"reboot") {
		m_info->TriggerType = TASK_EVENT_TRIGGER_AT_SYSTEMSTART;
	}
}

void WinTrigger::RepeatMin(DWORD every, DWORD forwhile) {
	m_info->MinutesInterval = every;
	m_info->MinutesDuration = forwhile;
}

void WinTrigger::RepeatWeek(const AutoUTF &interval, const AutoUTF &in) {
	if (interval.empty() || in.empty())
		return;
	DWORD inter = AsUInt(interval.c_str());
	DWORD dow = BitMask<DWORD>::FromStr(in, 7);
	if (WinBit::Check(dow, 6)) {
		WinBit::UnSet(dow, 6);
		dow = dow << 1;
		WinBit::Set(dow, 0);
	} else {
		dow = dow << 1;
	}

	if ((dow == 0) || (inter == 0))
		return;
	m_info->TriggerType = TASK_TIME_TRIGGER_WEEKLY;
	m_info->Type.Weekly.WeeksInterval = inter;
	m_info->Type.Weekly.rgfDaysOfTheWeek = dow;
}

void WinTrigger::RepeatMonth(const AutoUTF &in, const AutoUTF &day) {
	if (day.empty() || in.empty())
		return;
	DWORD mon = BitMask<DWORD>::FromStr(in, 12);
	DWORD dom = BitMask<DWORD>::FromStr(day, 31);
	if ((mon == 0) || (dom == 0))
		return;
	m_info->TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;
	m_info->Type.MonthlyDate.rgfMonths = mon;
	m_info->Type.MonthlyDate.rgfDays = dom;
}

DWORD WinTrigger::GetStartHour() const {
	return m_info->wStartHour;
}

DWORD WinTrigger::GetStartMinute() const {
	return m_info->wStartMinute;
}

DWORD WinTrigger::GetType() const {
	return m_info->TriggerType;
}

DWORD WinTrigger::GetDailyInterval() const {
	return m_info->Type.Daily.DaysInterval;
}

DWORD WinTrigger::GetWeeklyDow() const {
	DWORD tmp = m_info->Type.Weekly.rgfDaysOfTheWeek;
	if (WinBit::Check(tmp, 0)) {
		tmp = tmp >> 1;
		WinBit::Set(tmp, 6);
	} else {
		tmp = tmp >> 1;
	}
	return tmp;
}

DWORD WinTrigger::GetWeeklyInterval() const {
	return m_info->Type.Weekly.WeeksInterval;
}

DWORD WinTrigger::GetMonthlyMonth() const {
	return m_info->Type.MonthlyDate.rgfMonths;
}

DWORD WinTrigger::GetMonthlyDom() const {
	return m_info->Type.MonthlyDate.rgfDays;
}

DWORD WinTrigger::GetRepeatHour() const {
	return m_info->MinutesInterval / 60;
}

DWORD WinTrigger::GetRepeatMin() const {
	return m_info->MinutesInterval - (m_info->MinutesInterval / 60) * 60;
}

DWORD WinTrigger::GetRepeatDuraHour() const {
	return m_info->MinutesDuration / 60;
}

DWORD WinTrigger::GetRepeatDuraMin() const {
	return m_info->MinutesDuration - (m_info->MinutesDuration / 60) * 60;
}

bool WinTrigger::IsRepeat() const {
	return (m_info->MinutesInterval != 0) && (m_info->MinutesDuration != 0);
}

AutoUTF WinTrigger::GetPeriod() const {
	AutoUTF Result = L"custom";
	if (m_info->wRandomMinutesInterval == 1)
		Result = L"custom";
	else if ((m_info->TriggerType == TASK_TIME_TRIGGER_DAILY) && (m_info->MinutesInterval == 60))
		Result = L"hourly";
	else if (m_info->TriggerType == TASK_TIME_TRIGGER_DAILY)
		Result = L"daily";
	else if (m_info->TriggerType == TASK_TIME_TRIGGER_WEEKLY)
		Result = L"weekly";
	else if (m_info->TriggerType == TASK_TIME_TRIGGER_MONTHLYDATE)
		Result = L"monthly";
	else if (m_info->TriggerType == TASK_TIME_TRIGGER_MONTHLYDOW)
		Result = L"yearly";
	else if (m_info->TriggerType == TASK_EVENT_TRIGGER_AT_SYSTEMSTART)
		Result = L"reboot";
	return Result;
}

AutoUTF WinTrigger::AsStr() const {
	return m_str;
}

void WinTrigger::Init() {
	ZeroMemory(m_info.get(), sizeof(*m_info.get()));
	m_info->cbTriggerSize = sizeof(*m_info.get());
	time_t tTime = time(nullptr);
	tm *lt = localtime(&tTime);
	m_info->wBeginDay = lt->tm_mday;
	m_info->wBeginMonth = lt->tm_mon + 1;
	m_info->wBeginYear = lt->tm_year + 1900;
}

///========================================================================================= WinTask
WinTask::~WinTask() {
	m_pTask->Release();
}

WinTask::WinTask(const AutoUTF &name, bool autocreate) {
	try {
		CheckApiError(
			WinScheduler::instance()->Activate(name.c_str(), IID_ITask, (IUnknown**)&m_pTask));
	} catch (WinError &e) {
		if (autocreate && e.code() == 0x80070002) {//COR_E_FILENOTFOUND
			CheckApiError(WinScheduler::instance()->NewWorkItem(name.c_str(), // Name of task
						  CLSID_CTask, // Class identifier
						  IID_ITask, // Interface identifier
						  (IUnknown**)&m_pTask)); // Address of task interface
			return;
		}
		throw;
	}

}

void WinTask::Run() const {
	CheckApiError(m_pTask->Run());
}

void WinTask::Disable() {
	SetFlags(TASK_FLAG_DISABLED, true);
}

void WinTask::Enable() {
	SetFlags(TASK_FLAG_DISABLED, false);
}

void WinTask::SetFlags(DWORD in, bool on) {
	DWORD flags = GetFlags();
	WinFlag::Switch(flags, in, on);
	CheckApiError(m_pTask->SetFlags(flags));
}

void WinTask::SetAccount(const AutoUTF &name, PCWSTR pass) {
	CheckApiError(m_pTask->SetAccountInformation(name.c_str(), pass));
}

void WinTask::SetApplication(const AutoUTF &path) {
	CheckApiError(m_pTask->SetApplicationName(path.c_str()));
}

void WinTask::SetCreator(const AutoUTF &name) {
	CheckApiError(m_pTask->SetCreator(name.c_str()));
}

void WinTask::SetParam(const AutoUTF &in) {
	CheckApiError(m_pTask->SetParameters(in.c_str()));
}

void WinTask::SetWorkdir(const AutoUTF &path) {
	CheckApiError(m_pTask->SetWorkingDirectory(path.c_str()));
}

void WinTask::SetComment(const AutoUTF &in) {
	CheckApiError(m_pTask->SetComment(in.c_str()));
}

void WinTask::SetMaxRunTime(DWORD in) {
	CheckApiError(m_pTask->SetMaxRunTime(in));
}

void WinTask::AddTrigger(const WinTrigger &in) {
	ITaskTrigger *pITaskTrigger = nullptr;
	WORD index;
	CheckApiError(m_pTask->CreateTrigger(&index, &pITaskTrigger));
	try {
		CheckApiError(pITaskTrigger->SetTrigger(in.Info()));
	} catch (...) {
		pITaskTrigger->Release();
		throw;
	}
	pITaskTrigger->Release();
}

void WinTask::SetTrigger(DWORD index, const WinTrigger &in) {
	ITaskTrigger *pITaskTrigger = nullptr;
	CheckApiError(m_pTask->GetTrigger(index, &pITaskTrigger));
	try {
		CheckApiError(pITaskTrigger->SetTrigger(in.Info()));
	} catch (...) {
		pITaskTrigger->Release();
		throw;
	}
	pITaskTrigger->Release();
}

void WinTask::CleanTriggers() {
	WORD triggerCount = 0;
	CheckApiError(m_pTask->GetTriggerCount(&triggerCount));
	for (DWORD i = 0; i < triggerCount; ++i) {
		CheckApiError(m_pTask->DeleteTrigger(0));
	}
}

void WinTask::Save() const {
	IPersistFile *pPersistFile = nullptr;
	CheckApiError(m_pTask->QueryInterface(IID_IPersistFile, (PVOID*)&pPersistFile));
	//		pTask->Release();

	CheckApiError(pPersistFile->Save(nullptr, true));
	pPersistFile->Release();
}

DWORD WinTask::GetFlags() const {
	DWORD Result = 0;
	m_pTask->GetFlags(&Result);
	return Result;
}

AutoUTF WinTask::GetAccount() const {
	PWSTR str = nullptr;
	CheckApiError(m_pTask->GetAccountInformation(&str));
	//		if (err == SCHED_E_ACCOUNT_INFORMATION_NOT_SET)
	//			return	L"SYSTEM";
	AutoUTF Result(str);
	::CoTaskMemFree(str);
	return Result;
}

AutoUTF WinTask::GetApplication() const {
	PWSTR str = nullptr;
	CheckApiError(m_pTask->GetApplicationName(&str));
	AutoUTF Result(str);
	::CoTaskMemFree(str);
	return Result;
}

AutoUTF WinTask::GetCreator() const {
	PWSTR str = nullptr;
	CheckApiError(m_pTask->GetCreator(&str));
	AutoUTF Result(str);
	::CoTaskMemFree(str);
	return Result;
}

AutoUTF WinTask::GetParam() const {
	PWSTR str = nullptr;
	CheckApiError(m_pTask->GetParameters(&str));
	AutoUTF Result(str);
	::CoTaskMemFree(str);
	return Result;
}

AutoUTF WinTask::GetWorkdir() const {
	PWSTR str = nullptr;
	CheckApiError(m_pTask->GetWorkingDirectory(&str));
	AutoUTF Result(str);
	::CoTaskMemFree(str);
	return Result;
}

AutoUTF WinTask::GetComment() const {
	PWSTR str;
	CheckApiError(m_pTask->GetComment(&str));
	AutoUTF Result(str);
	::CoTaskMemFree(str);
	return Result;
}

bool WinTask::IsExist(const AutoUTF &name) {
	try {
		WinTask(name, false);
		return true;
	} catch (WinError &e) {
		if (e.code() != 0x80070002) //COR_E_FILENOTFOUND
			throw;
	}
	return false;
}

bool WinTask::IsDisabled(const AutoUTF &name) {
	return (GetFlags(name) & TASK_FLAG_DISABLED) ? true : false;
}

bool WinTask::IsRunning(const AutoUTF &name) {
	return (GetState(name) == SCHED_S_TASK_RUNNING) ? true : false;
}

void WinTask::Del(const AutoUTF &name) {
	CheckApiError(WinScheduler::instance()->Delete(name.c_str()));
}

DWORD WinTask::GetFlags(const AutoUTF &name) {
	return WinTask(name).GetFlags();
}

HRESULT WinTask::GetState(const AutoUTF &name) {
	HRESULT Result = 0;
	WinTask(name)->GetStatus(&Result);
	return Result;
}

AutoUTF WinTask::ParseState(HRESULT in) {
	switch (in) {
		case SCHED_S_TASK_READY:
			return L"The work item is ready to run at its next scheduled time";
		case SCHED_S_TASK_RUNNING:
			return L"The work item is currently running";
		case SCHED_S_TASK_NOT_SCHEDULED:
			return L"One or more of the properties that are needed to run this task on a schedule have not been set";
		case SCHED_S_TASK_HAS_NOT_RUN:
			return L"The task has not been run";
		case SCHED_S_TASK_DISABLED:
			return L"The task will not run at the scheduled times because it has been disabled";
		case SCHED_S_TASK_NO_MORE_RUNS:
			return L"There are no more runs scheduled for this task";
		case SCHED_S_TASK_NO_VALID_TRIGGERS:
			return L"Either the task has no triggers or the existing triggers are disabled or not set";
	}
	return L"Unknown State";
}

void WinTask::ParseCommand(const AutoUTF &comm, AutoUTF &app, AutoUTF &par) {
	app = L"";
	par = L"";
	size_t sp = comm.find(L" ");
	if (sp == AutoUTF::npos) {
		app = comm;
	} else {
		size_t qu1 = comm.find(L"\"");
		size_t qu2 = comm.find(L"\"", qu1 + 1);
		if (qu2 != AutoUTF::npos)
			++qu2;
		if (qu1 == 0) {
			app = comm.substr(0, qu2);
			par = comm.substr(qu2);
		} else {
			app = comm.substr(0, sp);
			par = comm.substr(sp);
		}
	}
}

///===================================================================================== WinTriggers
WinTriggers::~WinTriggers() {
	Clear();
}

WinTriggers::WinTriggers(const WinTask &task, bool autocache) :
		m_task(task) {
	if (autocache)
		Cache();
}

void WinTriggers::Cache() {
	WORD triggerCount = 0;
	CheckApiError(m_task->GetTriggerCount(&triggerCount));

	Clear();
	for (DWORD i = 0; i < triggerCount; ++i) {
		PWSTR str = nullptr;
		CheckApiError(m_task->GetTriggerString(i, &str));
		winstd::shared_ptr<WinTrigger> trg(new WinTrigger(str));
		ITaskTrigger *pTaskTrigger = nullptr;
		CheckApiError(m_task->GetTrigger(i, &pTaskTrigger));
		CheckApiError(pTaskTrigger->GetTrigger(trg->Info()));
		Insert(i, trg);
	}
}

WinTrigger& WinTriggers::Get() {
	return *(Value().get());
}

DWORD WinTriggers::GetStartHour() const {
	return Value()->GetStartHour();
}

DWORD WinTriggers::GetStartMinute() const {
	return Value()->GetStartMinute();
}

DWORD WinTriggers::GetType() const {
	return Value()->GetType();
}

AutoUTF WinTriggers::GetDailyInterval() const {
	return Num2Str(Value()->GetDailyInterval());
}

AutoUTF WinTriggers::GetWeeklyDow() const {
	return BitMask<DWORD>::AsStr(Value()->GetWeeklyDow(), 7);
}

AutoUTF WinTriggers::GetWeeklyInterval() const {
	return Num2Str(Value()->GetWeeklyInterval());
}

AutoUTF WinTriggers::GetMonthlyMonth() const {
	return BitMask<DWORD>::AsStr(Value()->GetMonthlyMonth(), 12);
}

AutoUTF WinTriggers::GetMonthlyDom() const {
	return BitMask<DWORD>::AsStr(Value()->GetMonthlyDom(), 31);
}

AutoUTF WinTriggers::GetRepeatHour() const {
	return Num2Str(Value()->GetRepeatHour());
}

AutoUTF WinTriggers::GetRepeatMin() const {
	return Num2Str(Value()->GetRepeatMin());
}

AutoUTF WinTriggers::GetRepeatDuraHour() const {
	return Num2Str(Value()->GetRepeatDuraHour());
}

AutoUTF WinTriggers::GetRepeatDuraMin() const {
	return Num2Str(Value()->GetRepeatDuraMin());
}

bool WinTriggers::IsRepeat() const {
	return Value()->IsRepeat();
}

AutoUTF WinTriggers::AsStr() const {
	return Value()->AsStr();
}

AutoUTF WinTriggers::AsStrAll(const AutoUTF &delim) {
	AutoUTF Result;
	ForEachIn(this) {
		Result += this->Value()->AsStr();
		Result += delim;
	}
	size_t pos = Result.rfind(delim);
	if (pos != AutoUTF::npos)
		Result.erase(pos);
	return Result;
}

///============================================================================================ Cron
TaskInfo::TaskInfo() :
		flags(0) {
}

TaskInfo::TaskInfo(const WinTask &task) {
	app = task.GetApplication();
	par = task.GetParam();
	path = task.GetWorkdir();
	comm = task.GetComment();
//	acc = task.GetAccount();
	creator = task.GetCreator();
}

void Cron::CacheByCreator(const AutoUTF &in) {
	IEnumWorkItems *pIEnum = nullptr;
	CheckApiError(WinScheduler::instance()->Enum(&pIEnum));
	m_creator = in;
	Clear();
	DWORD dwFetchedTasks = 0;
	PWSTR *lpwszNames;
	while (SUCCEEDED(pIEnum->Next(5, &lpwszNames, &dwFetchedTasks)) && (dwFetchedTasks != 0)) {
		while (dwFetchedTasks) {
			AutoUTF name = lpwszNames[--dwFetchedTasks];
			name.erase(name.rfind(L".job"));
			//			if (WinTask::GetFlags(name) & 0x200000) {
			//				::CoTaskMemFree(lpwszNames[dwFetchedTasks]);
			//				continue;
			//			}
			TaskInfo info(name);
			if (m_creator == info.creator || m_creator.empty())
				Insert(name, info);
			::CoTaskMemFree(lpwszNames[dwFetchedTasks]);
		}
		::CoTaskMemFree(lpwszNames);
	}
	pIEnum->Release();
}

void Cron::Add(const AutoUTF &name) {
	WinTask task(name);
	task.SetCreator(m_creator);
	task.Save();
	TaskInfo info(name);
	Insert(name, info);
}

void Cron::Del() {
	WinTask::Del(Key());
	Erase();
}

void Cron::Disable() {
	WinTask task(Key());
	task.Disable();
	task.Save();
}

void Cron::Enable() {
	WinTask task(Key());
	task.Enable();
	task.Save();
}

void Cron::SetApp(const AutoUTF &in) {
	WinTask task(Key());
	task.SetApplication(in);
	task.Save();
	Value().app = in;
}

void Cron::SetCreator(const AutoUTF &in) {
	WinTask task(Key());
	task.SetCreator(in);
	task.Save();
	Value().creator = in;
}

void Cron::SetParam(const AutoUTF &in) {
	WinTask task(Key());
	task.SetParam(in);
	task.Save();
	Value().par = in;
}

void Cron::SetPath(const AutoUTF &in) {
	WinTask task(Key());
	task.SetWorkdir(in);
	task.Save();
	Value().path = in;
}

void Cron::SetComm(const AutoUTF &in) {
	WinTask task(Key());
	task.SetComment(in);
	task.Save();
	Value().comm = in;
}

void Cron::SetAcc(const AutoUTF &name, PCWSTR pass) {
	WinTask task(Key());
	task.SetAccount(name, pass);
	task.Save();
}

void Cron::TriggerAdd(const WinTrigger &in) const {
	WinTask task(Key());
	task.AddTrigger(in);
	task.Save();
}

void Cron::TriggerClean() const {
	WinTask task(Key());
	task.CleanTriggers();
	task.Save();

}

DWORD Cron::GetFlags() const {
	return Value().flags;
}

AutoUTF Cron::GetName() const {
	return Key();
}

AutoUTF Cron::GetApp() const {
	return Value().app;
}

AutoUTF Cron::GetCreator() const {
	return Value().creator;
}

AutoUTF Cron::GetParam() const {
	return Value().par;
}

AutoUTF Cron::GetPath() const {
	return Value().path;
}

AutoUTF Cron::GetComm() const {
	return Value().comm;
}

AutoUTF Cron::GetTriggersString(const AutoUTF &delim) const {
	WinTriggers wts(WinTask(Key(), false));
	return wts.AsStrAll(delim);
}
