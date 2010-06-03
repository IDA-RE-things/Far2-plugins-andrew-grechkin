/**
	win_net_task
	manipulate Task Scheduler
	@classes	(WinScheduler, WinTriggers, WinTask, Cron)
	@author		Copyright © 2009 Grechkin Andrew
	@link		(ole32, uuid)
**/

#include "win_com.h"

///===================================================================================== WinTriggers
WinTriggers::WinTriggers(const AutoUTF &task, bool autocache): _task(task) {
	if (autocache)
		Cache();
}
bool					WinTriggers::Cache(const AutoUTF &task) {
	_task = (task.empty()) ? _task : task;
	ITask			*pTask = WinScheduler::GetTask(_task.c_str());
	ITaskTrigger	*pTaskTrigger = NULL;
	WORD			plTriggerCount = 0;

	if (!pTask) {
		return	false;
	}
	HRESULT err = pTask->GetTriggerCount(&plTriggerCount);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::GetTriggerCount";
	}
	TriggerInfo *pTrigger = new TriggerInfo;

	Clear();
	for (DWORD i = 0; i < plTriggerCount; ++i) {
		pTask->GetTrigger(i, &pTaskTrigger);
		if (FAILED(err)) {
			delete pTrigger;
			pTask->Release();
			throw "Failed calling ITask::GetTrigger";
		} else {
			pTaskTrigger->GetTrigger(pTrigger);
			Insert(i, *pTrigger);
		}
	}
	delete pTrigger;
	pTask->Release();
	Begin();
	return	(true);
}
bool					WinTriggers::Add(const TriggerInfo &in) {
	ITask			*pTask = WinScheduler::GetTask(_task.c_str());
	if (!pTask) {
		return	false;
	}
	ITaskTrigger *pITaskTrigger;
	WORD piNewTrigger;
	HRESULT err = pTask->CreateTrigger(&piNewTrigger, &pITaskTrigger);
	if (FAILED(err)) {
		pTask->Release();
		return	false;
	}
	/*
		TASK_TRIGGER tmp;
		ZeroMemory(&tmp, sizeof(TASK_TRIGGER));
		tmp.cbTriggerSize	= sizeof(TASK_TRIGGER);
	 	tmp.wBeginDay		= in.wBeginDay;
		tmp.wBeginMonth		= in.wBeginMonth;
		tmp.wBeginYear		= in.wBeginYear;
		tmp.wStartHour		= in.wStartHour;
		tmp.TriggerType		= in.TriggerType;
		tmp.Type			= in.Type;
	*/
	err = pITaskTrigger->SetTrigger(const_cast<TriggerInfo*>(&in));
	if (FAILED(err)) {
		pTask->Release();
		pITaskTrigger->Release();
		return	false;
	}

	pITaskTrigger->Release();
	WinTask::StopChanging(pTask);
	Insert(piNewTrigger, in);
	return	true;
}
bool					WinTriggers::Set(const TriggerInfo &in) {
	if (!ValidPtr())
		return	false;
	ITask			*pTask = WinScheduler::GetTask(_task.c_str());
	if (!pTask) {
		return	false;
	}
	ITaskTrigger *pITaskTrigger;
	HRESULT err = pTask->GetTrigger(Key(), &pITaskTrigger);
	if (FAILED(err)) {
		pTask->Release();
		return	false;
	}
	err = pITaskTrigger->SetTrigger(const_cast<TriggerInfo*>(&in));
	if (FAILED(err)) {
		pTask->Release();
		pITaskTrigger->Release();
		return	false;
	}
	pITaskTrigger->Release();
	WinTask::StopChanging(pTask);
	Value() = in;
	return	true;
}
bool					WinTriggers::Get(TriggerInfo &out) {
	bool Result = false;
	if (ValidPtr()) {
		out = Value();
		Result = true;
	}
	return	(Result);
}
bool					WinTriggers::Clean() {
	ITask			*pTask = WinScheduler::GetTask(_task.c_str());
	if (!pTask) {
		return	false;
	}
	while (!this->Empty()) {
		pTask->DeleteTrigger(0);
		this->Begin();
		Erase();
	}
	WinTask::StopChanging(pTask);
	return	true;
}
AutoUTF					WinTriggers::GetPeriod() {
	AutoUTF	Result = L"custom";
	if (ValidPtr()) {
		TriggerInfo *info = &(Value());
		if (info->wRandomMinutesInterval == 1)
			Result = L"custom";
		else if ((info->TriggerType == TASK_TIME_TRIGGER_DAILY) && (info->MinutesInterval == 60))
			Result = L"hourly";
		else if (info->TriggerType == TASK_TIME_TRIGGER_DAILY)
			Result = L"daily";
		else if (info->TriggerType == TASK_TIME_TRIGGER_WEEKLY)
			Result = L"weekly";
		else if (info->TriggerType == TASK_TIME_TRIGGER_MONTHLYDATE)
			Result = L"monthly";
		else if (info->TriggerType == TASK_TIME_TRIGGER_MONTHLYDOW)
			Result = L"yearly";
		else if (info->TriggerType == TASK_EVENT_TRIGGER_AT_SYSTEMSTART)
			Result = L"reboot";
	}
	return	Result;
}

///==================================== WinTriggersString ==========================================
WinTriggersString::WinTriggersString(const AutoUTF &task) {
	WORD			plTriggerCount = 0;
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask)
		return;

	HRESULT err = pTask->GetTriggerCount(&plTriggerCount);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::GetTriggerCount";
	}

	Clear();
	PWSTR ppwszTrigger = NULL;
	for (DWORD i = 0; i < plTriggerCount; ++i) {
		pTask->GetTriggerString(i, &ppwszTrigger);
		if (FAILED(err)) {
			pTask->Release();
			throw "Failed calling ITask::GetTrigger";
		} else {
			AutoUTF sTrigger = ppwszTrigger;
			Insert(sTrigger);
		}
	}
	if (plTriggerCount > 0)
		CoTaskMemFree(ppwszTrigger);
	pTask->Release();
}
AutoUTF					WinTriggersString::GetAll(const AutoUTF &in) {
	AutoUTF Result = L"";
	ForEachIn(this) {
		Result += this->GetString();
		Result += in;
	}
	size_t pos = Result.rfind(in);
	if (pos != string::npos)
		Result.erase(pos);
	return	Result;
}

///==================================== WinTask ====================================================
void					WinTask::CreateTrigger(const AutoUTF &task) {
	/*	ITask			*pTask = NULL;
		ITaskTrigger	*pTaskTrigger = NULL;
		WORD piNewTrigger = 0;
		StartChanging(task, pTask);
		HRESULT err = pTask->CreateTrigger(&piNewTrigger, &pTaskTrigger);
		if (FAILED(err)) {
			pTask->Release();
			throw "Failed calling ITask::CreateTrigger";
		}

		TASK_TRIGGER pTrigger;
		::ZeroMemory(&pTrigger, sizeof (TASK_TRIGGER));

		pTrigger.wBeginDay =1;
		pTrigger.wBeginMonth =1;
		pTrigger.wBeginYear =1999;
		pTrigger.cbTriggerSize = sizeof (TASK_TRIGGER);
		pTrigger.wStartHour = 13;
		pTrigger.TriggerType = TASK_TIME_TRIGGER_DAILY;
		pTrigger.Type.Daily.DaysInterval = 1;

		err = pTaskTrigger->SetTrigger (&pTrigger);
		if (FAILED(err)) {
			pTask->Release();
			pTaskTrigger->Release();
			throw "Failed calling ITaskTrigger::SetTrigger";
		}
	*/
}
void					WinTask::LoadTrigger(const AutoUTF &task, WinTriggers &nt) {
	/*	ITask			*pTask = NULL;
		ITaskTrigger	*pTaskTrigger = NULL;
		WORD			plTriggerCount = 0;
		StartChanging(task, pTask);
		HRESULT err = pTask->GetTriggerCount (&plTriggerCount);
		if (FAILED(err)) {
			pTask->Release();
			throw "Failed calling ITask::GetTriggerCount";
		}
		TASK_TRIGGER* pTrigger = new TASK_TRIGGER;

		for (DWORD i = 0; i < plTriggerCount; ++i) {
			pTask->GetTrigger(i, &pTaskTrigger);
			if (FAILED(err)) {
				delete pTrigger;
				pTask->Release();
				throw "Failed calling ITask::GetTrigger";
			} else {
				pTaskTrigger->GetTrigger(pTrigger);
	//			nt.Insert(*pTrigger);
			}
		}
		delete pTrigger;
		pTask->Release();*/
}

// static
bool					WinTask::IsExist(const AutoUTF &task) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	return	(pTask != NULL) ? pTask->Release(), true : false;
}
DWORD					WinTask::GetFlags(const AutoUTF &task) {
	DWORD			Result = 0;
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	Result;
	}
	HRESULT	err = pTask->GetFlags(&Result);
	pTask->Release();
	if (FAILED(err)) {
		throw "Failed calling ITask::GetFlags";
	}
	return	Result;
}
HRESULT					WinTask::GetState(const AutoUTF &task) {
	HRESULT			Result = S_FALSE;
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	Result;
	}
	HRESULT	err = pTask->GetStatus(&Result);
	pTask->Release();
	if (FAILED(err)) {
		return	err;
	}
	return	Result;
}

void					WinTask::StopChanging(ITask *pTask) {
	IPersistFile	*pPersistFile = NULL;

	HRESULT err = pTask->QueryInterface(IID_IPersistFile, (PVOID*) & pPersistFile);
	pTask->Release();

	err = pPersistFile->Save(NULL, true);
	if (SUCCEEDED(err))
		pPersistFile->Release();
	if (FAILED(err))
		throw "Failed calling IPersistFile::Save";
}
bool					WinTask::Add(const AutoUTF &task) {
	if (IsExist(task))
		return	false;
	ITaskScheduler	*pITS =  WinScheduler::the();
	ITask			*pTask = NULL;
	if (!pITS) {
		return	false;
	}
	HRESULT err = pITS->NewWorkItem(task.c_str(),				// Name of task
									CLSID_CTask,				// Class identifier
									IID_ITask,					// Interface identifier
									(IUnknown**) & pTask);		// Address of task interface
	if (FAILED(err))
		return	false;
	StopChanging(pTask);
	return	true;
}
bool					WinTask::Del(const AutoUTF &task) {
	bool			Result = false;
	ITaskScheduler	*pITS = WinScheduler::the();
	if (pITS && IsExist(task))
		Result = SUCCEEDED(pITS->Delete(task.c_str()));
	return	Result;
}
bool					WinTask::SetAcc(const AutoUTF &task, const AutoUTF &name, PCWSTR pass) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	false;
	}
	if (FAILED(pTask->SetAccountInformation(name.c_str(), pass))) {
		pTask->Release();
		return	false;
	}
	StopChanging(pTask);
	return	true;
}
bool					WinTask::SetApp(const AutoUTF &task, const AutoUTF &in) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	false;
	}
	if (FAILED(pTask->SetApplicationName(in.c_str()))) {
		pTask->Release();
		return	false;
	}
	StopChanging(pTask);
	return	true;
}
bool					WinTask::SetCreator(const AutoUTF &task, const AutoUTF &in) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	false;
	}
	if (FAILED(pTask->SetCreator(in.c_str()))) {
		pTask->Release();
		return	false;
	}
	StopChanging(pTask);
	return	true;
}
bool					WinTask::SetPar(const AutoUTF &task, const AutoUTF &in) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	false;
	}
	if (FAILED(pTask->SetParameters(in.c_str()))) {
		pTask->Release();
		return	false;
	}
	StopChanging(pTask);
	return	true;
}
bool					WinTask::SetPath(const AutoUTF &task, const AutoUTF &in) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	false;
	}
	if (FAILED(pTask->SetWorkingDirectory(in.c_str()))) {
		pTask->Release();
		return	false;
	}
	StopChanging(pTask);
	return	true;
}
void					WinTask::SetComm(const AutoUTF &task, const AutoUTF &in) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return;
	}
	if (FAILED(pTask->SetComment(in.c_str()))) {
		pTask->Release();
		throw "Failed calling ITask::SetComment";
	}
	StopChanging(pTask);
}
void					WinTask::SetMaxRunTime(const AutoUTF &task, DWORD in) {
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return;
	}
	if (FAILED(pTask->SetMaxRunTime(in))) {
		pTask->Release();
		throw "Failed calling ITask::SetMaxRunTime";
	}
	StopChanging(pTask);
}
void					WinTask::SetFlags(const AutoUTF &task, DWORD in, bool on) {
	DWORD			Flags = GetFlags(task);
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return;
	}
	(on) ? Flags |= in : Flags &= ~in;
	HRESULT err = pTask->SetFlags(Flags);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::SetFlags";
	}
	StopChanging(pTask);
}
void					WinTask::UnSetFlags(const AutoUTF &task, DWORD in) {
	SetFlags(task, in, false);
}
bool					WinTask::Run(const AutoUTF &task) {
	bool Result = false;
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	(Result);
	}
	HRESULT err = pTask->Run();
	pTask->Release();
	if (SUCCEEDED(err)) {
		Result = true;
	}
	return	(Result);
}
void					WinTask::Disable(const AutoUTF &task) {
	SetFlags(task, TASK_FLAG_DISABLED);
}
void					WinTask::Enable(const AutoUTF &task) {
	UnSetFlags(task, TASK_FLAG_DISABLED);
}
AutoUTF					WinTask::GetAcc(const AutoUTF &task) {
	AutoUTF			Result;
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	Result;
	}
	PWSTR	ppwszString = NULL;
	HRESULT err = pTask->GetAccountInformation(&ppwszString);
	if (FAILED(err)) {
		pTask->Release();
		if (err == SCHED_E_ACCOUNT_INFORMATION_NOT_SET)
			return	L"SYSTEM";
		throw "Failed calling ITask::GetAccountInformation";
	}
	Result = ppwszString;
	::CoTaskMemFree(ppwszString);
	pTask->Release();
	return	Result;
}
AutoUTF					WinTask::GetApp(const AutoUTF &task) {
	AutoUTF			Result = L"";
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	(Result);
	}
	PWSTR ppwszString;
	HRESULT err = pTask->GetApplicationName(&ppwszString);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::GetApplicationName";
	}
	Result = ppwszString;
	CoTaskMemFree(ppwszString);
	pTask->Release();
	return	(Result);
}
AutoUTF					WinTask::GetCreator(const AutoUTF &task) {
	AutoUTF			Result = L"";
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (pTask) {
		PWSTR tmp;
		HRESULT err = pTask->GetCreator(&tmp);
		if (FAILED(err)) {
			pTask->Release();
			throw "Failed calling ITask::GetCreator";
		}
		Result = tmp;
		::CoTaskMemFree(tmp);
		pTask->Release();
	}
	return	Result;
}
AutoUTF					WinTask::GetPar(const AutoUTF &task) {
	AutoUTF			Result = L"";
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	(Result);
	}
	PWSTR ppwszString;
	HRESULT err = pTask->GetParameters(&ppwszString);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::GetParameters";
	}
	Result = ppwszString;
	CoTaskMemFree(ppwszString);
	pTask->Release();
	return	(Result);
}
AutoUTF					WinTask::GetPath(const AutoUTF &task) {
	AutoUTF			Result = L"";
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	(Result);
	}
	PWSTR ppwszString;
	HRESULT err = pTask->GetWorkingDirectory(&ppwszString);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::GetWorkingDirectory";
	}
	Result = ppwszString;
	CoTaskMemFree(ppwszString);
	pTask->Release();
	return	(Result);
}
AutoUTF					WinTask::GetComm(const AutoUTF &task) {
	AutoUTF			Result = L"";
	ITask			*pTask = WinScheduler::GetTask(task.c_str());
	if (!pTask) {
		return	(Result);
	}
	PWSTR ppwszString;
	HRESULT err = pTask->GetComment(&ppwszString);
	if (FAILED(err)) {
		pTask->Release();
		throw "Failed calling ITask::GetComment";
	}
	Result = ppwszString;
	CoTaskMemFree(ppwszString);
	pTask->Release();
	return	(Result);
}
AutoUTF					WinTask::ParseState(HRESULT in) {
	switch (in) {
		case SCHED_S_TASK_READY:
			return	L"The work item is ready to run at its next scheduled time";
		case SCHED_S_TASK_RUNNING:
			return	L"The work item is currently running";
		case SCHED_S_TASK_NOT_SCHEDULED:
			return	L"One or more of the properties that are needed to run this task on a schedule have not been set";
		case SCHED_S_TASK_HAS_NOT_RUN:
			return	L"The task has not been run";
		case SCHED_S_TASK_DISABLED:
			return	L"The task will not run at the scheduled times because it has been disabled";
		case SCHED_S_TASK_NO_MORE_RUNS:
			return	L"There are no more runs scheduled for this task";
		case SCHED_S_TASK_NO_VALID_TRIGGERS:
			return	L"Either the task has no triggers or the existing triggers are disabled or not set";
	}
	return	L"Unknown State";
}
void					WinTask::ParseCommand1(const AutoUTF &comm, AutoUTF &app, AutoUTF &par) {
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

///==================================== Cron =======================================================
bool					Cron::CacheByCreator(const AutoUTF &in) {
	bool	Result = false;
	_creator = in;
	ITaskScheduler *pITS = WinScheduler::the();
	IEnumWorkItems *pIEnum;

	HRESULT err = pITS->Enum(&pIEnum);
	if (FAILED(err)) {
		throw "Can`t enum tasks";
		return	Result;
	}

	Clear();
	DWORD	dwFetchedTasks = 0;
	PWSTR	*lpwszNames;
	while (SUCCEEDED(pIEnum->Next(5, &lpwszNames, &dwFetchedTasks)) && (dwFetchedTasks != 0)) {
		while (dwFetchedTasks) {
			AutoUTF	name = lpwszNames[--dwFetchedTasks];
			name.erase(name.rfind(L".job"));
			if (WinTask::GetFlags(name) & 0x200000) {
				::CoTaskMemFree(lpwszNames[dwFetchedTasks]);
				continue;
			}
			TaskInfo info;
			info.app	= WinTask::GetApp(name);
			info.par	= WinTask::GetPar(name);
			info.path	= WinTask::GetPath(name);
			info.comm	= WinTask::GetComm(name);
//			info.acc	= WinTask::GetAcc(name);
			info.creat	= WinTask::GetCreator(name);
			if (_creator == info.creat || _creator.empty())
				Insert(name, info);
			::CoTaskMemFree(lpwszNames[dwFetchedTasks]);
		}
		::CoTaskMemFree(lpwszNames);
	}
	pIEnum->Release();
	return	true;
}
bool					Cron::Add(const AutoUTF &name) {
	bool Result = false;
	if (!Exist(name)) {
		Result = WinTask::Add(name);
		if (Result) {
			TaskInfo info;
			info.app	= L"";
			info.path	= L"";
			info.comm	= L"";
			info.creat	= _creator;
			Insert(name, info);
		}
	}
	return	(Result);
}
bool					Cron::Del() {
	bool Result = false;
	if (ValidPtr()) {
		Result = WinTask::Del(Key());
		if (Result)
			Erase();
	}
	return	Result;
}
bool					Cron::Disable() {
	if (ValidPtr()) {
		WinTask::Disable(Key());
	}
	return	true;
}
bool					Cron::Enable() {
	if (ValidPtr()) {
		WinTask::Enable(Key());
	}
	return	true;
}
void					Cron::SetApp(const AutoUTF &in) {
	if (ValidPtr()) {
		if (WinTask::SetApp(Key(), in))
			Value().app = in;
	}
}
void					Cron::SetCreator(const AutoUTF &in) {
	if (ValidPtr()) {
		if (WinTask::SetCreator(Key(), in))
			Value().creat = in;
	}
}
void					Cron::SetPar(const AutoUTF &in) {
	if (ValidPtr()) {
		if (WinTask::SetPar(Key(), in))
			Value().par = in;
	}
}
void					Cron::SetPath(const AutoUTF &in) {
	if (ValidPtr()) {
		if (WinTask::SetPath(Key(), in))
			Value().path = in;
	}
}
void					Cron::SetComm(const AutoUTF &in) {
}
void					Cron::SetAcc(const AutoUTF &name, PCWSTR pass) {
	if (ValidPtr()) {
		WinTask::SetAcc(Key(), name, pass);
	}
}
void					Cron::TriggerAdd(const TriggerInfo &in) const {
	if (ValidPtr()) {
		WinTriggers wt(Key());
		wt.Add(in);
	}
}
void					Cron::TriggerClean() const {
	if (ValidPtr()) {
		WinTriggers wt(Key());
		wt.Clean();
	}
}
AutoUTF					Cron::GetName() const {
	return	(ValidPtr()) ? Key() : L"";
}
AutoUTF					Cron::GetApp() const {
	return	(ValidPtr()) ? Value().app : L"";
}
AutoUTF					Cron::GetCreator() const {
	return	(ValidPtr()) ? Value().creat : L"";
}
AutoUTF					Cron::GetPar() const {
	return	(ValidPtr()) ? Value().par : L"";
}
AutoUTF					Cron::GetPath() const {
	return	(ValidPtr()) ? Value().path : L"";
}
AutoUTF					Cron::GetComm() const {
	return	(ValidPtr()) ? Value().comm : L"";
}
AutoUTF					Cron::GetTriggersString(const AutoUTF &in) const {
	AutoUTF Result = L"";
	if (ValidPtr()) {
		WinTriggersString wts(Key());
		Result = wts.GetAll(in);
	}
	return	(Result);
}
DWORD					Cron::GetFlags() const {
	return	(ValidPtr()) ? WinTask::GetFlags(Key()) : 0;
}
