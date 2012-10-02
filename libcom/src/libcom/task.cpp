// include initguid always first
#include <initguid.h>

#include <libcom/std.hpp>
#include <libcom/mem.hpp>
#include <libcom/task.hpp>
#include <libext/exception.hpp>
#include <libext/time.hpp>
#include <libbase/bit.hpp>

#include <mstask.h>

#include <time.h>

namespace Com {
///==================================================================================== WinScheduler
struct WinScheduler: private Base::Uncopyable {
	ITaskScheduler* operator->() {
		return m_ts;
	}

	static WinScheduler& instance() {
		static WinScheduler sche;
		return sche;
	}

private:
	WinScheduler() {
		CheckApiError(::CoCreateInstance(CLSID_CTaskScheduler, nullptr, CLSCTX_INPROC_SERVER,
										 IID_ITaskScheduler, (PVOID*)&m_ts));
	}

	ComObject<ITaskScheduler> m_ts;
};

///====================================================================================== WinTrigger
WinTrigger::~WinTrigger() {
}

WinTrigger::WinTrigger(size_t index, const ComObject<ITaskTrigger> &trigger):
	m_trigger(trigger),
	m_info(new _TASK_TRIGGER),
	m_index(index) {
	CheckApiError(m_trigger->GetTrigger(m_info.get()));
}

void WinTrigger::set_time(DWORD h, DWORD m) {
	m_info->wStartHour = h;
	m_info->wStartMinute = m;
	save();
}

void WinTrigger::set_daily(size_t interval) {
	m_info->TriggerType = TASK_TIME_TRIGGER_DAILY;
	m_info->Type.Daily.DaysInterval = interval;
	save();
}

void WinTrigger::set_weekly(WORD days_of_week, size_t interval) {
	m_info->TriggerType = TASK_TIME_TRIGGER_WEEKLY;
	m_info->Type.Weekly.WeeksInterval = interval;
	m_info->Type.Weekly.rgfDaysOfTheWeek = days_of_week;
	save();
}

void WinTrigger::set_monthly(size_t days_bits, size_t months_bits) {
	m_info->TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;
	m_info->Type.MonthlyDate.rgfMonths = months_bits;
	m_info->Type.MonthlyDate.rgfDays = days_bits;
	save();
}

void WinTrigger::set_repeat(DWORD every, DWORD forwhile) {
	m_info->MinutesInterval = every;
	m_info->MinutesDuration = forwhile;
	save();
}

DWORD WinTrigger::get_starthour() const {
	return m_info->wStartHour;
}

DWORD WinTrigger::get_startminute() const {
	return m_info->wStartMinute;
}

DWORD WinTrigger::get_type() const {
	return m_info->TriggerType;
}

DWORD WinTrigger::get_daily_interval() const {
	return m_info->Type.Daily.DaysInterval;
}

DWORD WinTrigger::get_weekly_dow() const {
	return m_info->Type.Weekly.rgfDaysOfTheWeek;
}

DWORD WinTrigger::get_weekly_interval() const {
	return m_info->Type.Weekly.WeeksInterval;
}

DWORD WinTrigger::get_monthly_month() const {
	return m_info->Type.MonthlyDate.rgfMonths;
}

DWORD WinTrigger::get_monthly_dom() const {
	return m_info->Type.MonthlyDate.rgfDays;
}

DWORD WinTrigger::get_repeat() const {
	return m_info->MinutesInterval;
}

DWORD WinTrigger::get_repeat_duration() const {
	return m_info->MinutesDuration;
}

bool WinTrigger::is_repeat() const {
	return (m_info->MinutesInterval != 0) && (m_info->MinutesDuration != 0);
}

ustring WinTrigger::as_str() const {
	CoMem<PWSTR> str;
	CheckApiError(m_trigger->GetTriggerString(&str));
	return ustring(str);
}

//void WinTrigger::Init() {
//	ZeroMemory(m_info.get(), sizeof(*m_info.get()));
//	m_info->cbTriggerSize = sizeof(*m_info.get());
//	LocalTime time;
//	m_info->wBeginDay = time.day();
//	m_info->wBeginMonth = time.month();
//	m_info->wBeginYear = time.year();
//}

WinTrigger::WinTrigger(size_t index, const ComObject<ITask> &task):
	m_info(new _TASK_TRIGGER),
	m_index(index) {
	CheckApiError(task->GetTrigger(m_index, &m_trigger));
	CheckApiError(m_trigger->GetTrigger(m_info.get()));
}

void WinTrigger::save() {
	CheckApiError(m_trigger->SetTrigger(m_info.get()));
}

///========================================================================================= WinTask
WinTask::~WinTask() {
}

WinTask::WinTask(const ustring &name, bool autocreate):
	m_name(name) {
	try {
		CheckApiError(WinScheduler::instance()->Activate(name.c_str(), IID_ITask, (IUnknown**)&m_task));
	} catch (Ext::AbstractError & e) {
		if (autocreate && e.code() == (DWORD)HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {//COR_E_FILENOTFOUND
			CheckApiError(WinScheduler::instance()->NewWorkItem(name.c_str(),
						  CLSID_CTask, IID_ITask, (IUnknown**)&m_task));
			return;
		}
		throw;
	}
}

WinTask::operator ComObject<ITask>() const {
	return m_task;
}

ComObject<ITask> WinTask::operator->() const {
	return m_task;
}

void WinTask::run() const {
	CheckApiError(m_task->Run());
}

void WinTask::set_flags(DWORD in, bool on) {
	DWORD flags = get_flags();
	Base::WinFlag::Switch(flags, in, on);
	CheckApiError(m_task->SetTaskFlags(flags));
}

void WinTask::disable() {
	set_flags(TASK_FLAG_DISABLED, true);
}

void WinTask::enable() {
	set_flags(TASK_FLAG_DISABLED, false);
}

void WinTask::set_account(const ustring &name, PCWSTR pass) {
	CheckApiError(m_task->SetAccountInformation(name.c_str(), pass));
}

void WinTask::set_application(const ustring &path) {
	CheckApiError(m_task->SetApplicationName(path.c_str()));
}

void WinTask::set_creator(const ustring &name) {
	CheckApiError(m_task->SetCreator(name.c_str()));
}

void WinTask::set_param(const ustring &in) {
	CheckApiError(m_task->SetParameters(in.c_str()));
}

void WinTask::set_workdir(const ustring &path) {
	CheckApiError(m_task->SetWorkingDirectory(path.c_str()));
}

void WinTask::set_comment(const ustring &in) {
	CheckApiError(m_task->SetComment(in.c_str()));
}

void WinTask::set_max_runtime(DWORD in) {
	CheckApiError(m_task->SetMaxRunTime(in));
}

void WinTask::add_trigger(const WinTrigger &in) {
	ComObject<ITaskTrigger> pITaskTrigger;
	WORD index;
	CheckApiError(m_task->CreateTrigger(&index, &pITaskTrigger));
	CheckApiError(pITaskTrigger->SetTrigger(in));
}

void WinTask::set_trigger(DWORD index, const WinTrigger &in) {
	ComObject<ITaskTrigger> pITaskTrigger;
	CheckApiError(m_task->GetTrigger(index, &pITaskTrigger));
	CheckApiError(pITaskTrigger->SetTrigger(in));
}

void WinTask::clear_triggers() {
	WORD triggerCount = 0;
	CheckApiError(m_task->GetTriggerCount(&triggerCount));
	for (WORD i = triggerCount; i > 0; --i) {
		CheckApiError(m_task->DeleteTrigger(i - 1));
	}
}

void WinTask::save() const {
	ComObject<IPersistFile> persistFile;
	CheckApiError(m_task->QueryInterface(IID_IPersistFile, (PVOID*)&persistFile));
	CheckApiError(persistFile->Save(nullptr, true));
}

ustring WinTask::name() const {
	return m_name;
}

size_t WinTask::get_triggers_count() const {
	WORD triggerCount = 0;
	CheckApiError(m_task->GetTriggerCount(&triggerCount));
	return triggerCount;
}

DWORD WinTask::get_flags() const {
	DWORD Result = 0;
	CheckApiError(m_task->GetTaskFlags(&Result));
	return Result;
}

HRESULT WinTask::get_status() const {
	HRESULT Result = 0;
	CheckApiError(m_task->GetStatus(&Result));
	return Result;
}

ustring WinTask::get_account() const {
	CoMem<PWSTR> str;
	CheckApiError(m_task->GetAccountInformation(&str));
	//		if (err == SCHED_E_ACCOUNT_INFORMATION_NOT_SET)
	//			return L"SYSTEM";
	return ustring(str);
}

ustring WinTask::get_application() const {
	CoMem<PWSTR> str;
	CheckApiError(m_task->GetApplicationName(&str));
	return ustring(str);
}

ustring WinTask::get_creator() const {
	CoMem<PWSTR> str;
	CheckApiError(m_task->GetCreator(&str));
	return ustring(str);
}

ustring WinTask::get_param() const {
	CoMem<PWSTR> str;
	CheckApiError(m_task->GetParameters(&str));
	return ustring(str);
}

ustring WinTask::get_workdir() const {
	CoMem<PWSTR> str;
	CheckApiError(m_task->GetWorkingDirectory(&str));
	return ustring(str);
}

ustring WinTask::get_comment() const {
	CoMem<PWSTR> str;
	CheckApiError(m_task->GetComment(&str));
	return ustring(str);
}

DWORD WinTask::get_flags(const ustring &name) {
	return WinTask(name).get_flags();
}

HRESULT WinTask::get_state(const ustring &name) {
	return WinTask(name).get_status();
}

bool WinTask::is_exist(const ustring &name) {
	ComObject<ITask> m_task;
	return WinScheduler::instance()->Activate(name.c_str(), IID_ITask, (IUnknown**)&m_task) == 0;
}

bool WinTask::is_disabled(const ustring &name) {
	return get_flags(name) & TASK_FLAG_DISABLED;
}

bool WinTask::is_running(const ustring &name) {
	return get_state(name) == SCHED_S_TASK_RUNNING;
}

void WinTask::del(const ustring &name) {
	CheckApiError(WinScheduler::instance()->Delete(name.c_str()));
}

ustring WinTask::parse_state(HRESULT in) {
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

void WinTask::parse_command(const ustring &comm, ustring &app, ustring &par) {
	app = L"";
	par = L"";
	size_t sp = comm.find(L" ");
	if (sp == ustring::npos) {
		app = comm;
	} else {
		size_t qu1 = comm.find(L"\"");
		size_t qu2 = comm.find(L"\"", qu1 + 1);
		if (qu2 != ustring::npos)
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
WinTriggers::WinTriggers(const WinTask &task) :
	m_task(task) {
	cache();
}

void WinTriggers::cache() {
	size_t size(m_task.get_triggers_count());
	clear();
	for (DWORD i = 0; i < size; ++i) {
//		CoMem<PWSTR> str;
//		CheckApiError(m_task->GetTriggerString(i, &str));
		ComObject<ITaskTrigger> taskTrigger;
		CheckApiError(m_task->GetTrigger(i, &taskTrigger));
		push_back(WinTrigger(i, taskTrigger));
	}
}

ustring WinTriggers::AsStrAll(const ustring &delim) {
	ustring ret;
	for (iterator it = begin(); it != end(); ++it) {
		if (it != begin())
			ret += delim;
		ret += it->as_str();
	}
	return ret;
}

WinTriggers::iterator WinTriggers::find(size_t index) {
	return std::find(begin(), end(), index);
}

void WinTriggers::add(const WinTrigger &in) {
	WORD index;
	ComObject<ITaskTrigger> trigger;
	CheckApiError(m_task->CreateTrigger(&index, &trigger));
	CheckApiError(trigger->SetTrigger(in));
	push_back(in);
}

void WinTriggers::del(size_t index) {
	iterator it = find(index);
	if (it != end())
		del(it);
}

void WinTriggers::del(iterator it) {
	m_task->DeleteTrigger(std::distance(begin(), it));
	erase(it);
}

///==================================================================================== WinScheduler
WinTasks::WinTasks(bool autocache) :
	m_creator(ustring()) {
	if (autocache)
		cache();
}

void WinTasks::cache_by_creator(const ustring &creator) {
	ComObject<IEnumWorkItems> ienum;
	CheckApiError(WinScheduler::instance()->Enum(&ienum));
	m_creator = creator;
	clear();

	DWORD dwFetchedTasks = 0;
	CoMem<PWSTR*> names;
	while (SUCCEEDED(ienum->Next(5, &names, &dwFetchedTasks)) && (dwFetchedTasks != 0)) {
		while (dwFetchedTasks) {
			ustring name = names[--dwFetchedTasks];
			name.erase(name.rfind(L".job"));
			WinTask task(name);
			if (m_creator.empty() || m_creator == task.get_creator())
				push_back(task);
			::CoTaskMemFree(names[dwFetchedTasks]);
		}
	}
}

WinTasks::iterator WinTasks::find(const ustring &name) {
	return std::find(begin(), end(), name);
}

void WinTasks::Add(const ustring &name) {
	WinTask task(name);
	task.set_creator(m_creator);
	task.save();
	push_back(task);
}

void WinTasks::del(const ustring &name) {
	iterator it = find(name);
	if (it != end())
		del(it);
}

void WinTasks::del(iterator it) {
	WinTask::del(it->name());
	erase(it);
}

}
