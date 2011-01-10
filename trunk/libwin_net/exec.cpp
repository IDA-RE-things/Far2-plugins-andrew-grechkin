#include "win_net.h"

DWORD Exec::TIMEOUT = 20000;
DWORD Exec::TIMEOUT_DX = 200;

void Exec::Run(const AutoUTF &cmd) {
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	AutoUTF app = Validate(cmd);
	CheckApi(::CreateProcessW(nullptr, (PWSTR)app.c_str(), nullptr, nullptr, false,
			CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);
}

int Exec::Run(const AutoUTF &cmd, astring &out) {
	DWORD Result = 0;

	// Pipe for read stdout
	auto_close<HANDLE> hPipeOutRead, hPipeOutWrite;
//	WinHandle hPipeOutRead, hPipeOutWrite;
	CheckApi(::CreatePipe(&hPipeOutRead, &hPipeOutWrite, nullptr, 0));
	CheckApi(::SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	// fork process
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.hStdOutput = hPipeOutWrite;
	si.hStdError = hPipeOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	AutoUTF app = Validate(cmd);
	CheckApi(::CreateProcessW(nullptr, (PWSTR)app.c_str(), nullptr, nullptr, true,
			CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	hPipeOutWrite.close();
	::CloseHandle(pi.hThread);
	auto_close<HANDLE> hProc(pi.hProcess);
	DWORD timeout = TIMEOUT;
	out.clear();
	CHAR buf[1024 * 1024];
	DWORD dwRead;
	DWORD dwAvail = 0;
	bool childTerminate = false;
	while (!childTerminate && timeout > 0) {
		if (::WaitForSingleObject(hProc, TIMEOUT_DX) == WAIT_OBJECT_0) {
			childTerminate = true;
			::GetExitCodeProcess(hProc, &Result);
		} else {
			timeout -= TIMEOUT_DX;
		}
		while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
			while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
				WinMem::Zero(buf, sizeofa(buf));
				::ReadFile(hPipeOutRead, buf, sizeofa(buf), &dwRead, nullptr);
				out += buf;
			}
			::Sleep(TIMEOUT_DX / 20);
		}
	}
	if (timeout <= 0) {
		throw ApiError(WAIT_TIMEOUT);
	}
	return Result;
}

int Exec::Run(const AutoUTF &cmd, astring &out, const astring &in) {
	DWORD Result = 0;

	// Pipe for write stdin
	auto_close<HANDLE> hPipeInRead, hPipeInWrite;
	CheckApi(::CreatePipe(&hPipeInRead, &hPipeInWrite, nullptr, in.size() + 1));
	CheckApi(::SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	DWORD dwWritten = 0;
	CheckApi(::WriteFile(hPipeInWrite, in.c_str(), in.size(), &dwWritten, nullptr));

	// Pipe for read stdout
	auto_close<HANDLE> hPipeOutRead, hPipeOutWrite;
	CheckApi(::CreatePipe(&hPipeOutRead, &hPipeOutWrite, nullptr, 0));
	CheckApi(::SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	// fork process
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.hStdInput = hPipeInRead;
	si.hStdOutput = hPipeOutWrite;
	si.hStdError = hPipeOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	AutoUTF app = Validate(cmd);
	CheckApi(::CreateProcessW(nullptr, (PWSTR)app.c_str(), nullptr, nullptr, true,
			CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	hPipeInRead.close();
	hPipeInWrite.close();
	hPipeOutWrite.close();
	::CloseHandle(pi.hThread);

	auto_close<HANDLE> hProc(pi.hProcess);
	DWORD timeout = TIMEOUT;
	CHAR buf[1024 * 1024];
	DWORD dwRead;
	DWORD dwAvail = 0;
	bool childTerminate = false;
	out.clear();
	while (!childTerminate && timeout > 0) {
		if (::WaitForSingleObject(hProc, TIMEOUT_DX) == WAIT_OBJECT_0) {
			childTerminate = true;
			::GetExitCodeProcess(hProc, &Result);
		} else {
			timeout -= TIMEOUT_DX;
		}
		while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
			while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
				WinMem::Zero(buf, sizeof(buf));
				::ReadFile(hPipeOutRead, buf, sizeof(buf), &dwRead, nullptr);
				out += buf;
			}
			::Sleep(TIMEOUT_DX / 20);
		}
	}
	if (timeout <= 0) {
		throw ApiError(WAIT_TIMEOUT);
	}
	return Result;
}

int Exec::RunWait(const AutoUTF &cmd, DWORD wait) {
	DWORD Result = 0;

	// fork process
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	AutoUTF app = Validate(cmd);
	CheckApi(::CreateProcessW(nullptr, (PWSTR)app.c_str(), nullptr, nullptr, true,
			CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	::CloseHandle(pi.hThread);
	auto_close<HANDLE> hProc(pi.hProcess);
	if (::WaitForSingleObject(hProc, wait) == WAIT_OBJECT_0) {
		::GetExitCodeProcess(hProc, &Result);
	} else {
		throw ApiError(WAIT_TIMEOUT);
	}
	return Result;
}

void Exec::RunAsUser(const AutoUTF &cmd, HANDLE hToken) {
	AutoUTF app = Validate(cmd);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);

	CheckApi(::CreateProcessAsUserW(hToken, nullptr, (PWSTR)app.c_str(), nullptr, nullptr, false, CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);
}

void Exec::RunAsUser(const AutoUTF &cmd, const AutoUTF &user, const AutoUTF &pass) {
	AutoUTF app = Validate(cmd);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);

	CheckApi(::CreateProcessWithLogonW(user.c_str(), nullptr, pass.c_str(), 0, nullptr, (PWSTR)app.c_str(), CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);
}

int Exec::RunAsUser(const AutoUTF &cmd, astring &out, const astring &in, const AutoUTF &user,
                    const AutoUTF &pass) {
	DWORD Result = 0;

	// Pipe for write stdin
	auto_close<HANDLE> hPipeInRead, hPipeInWrite;
	CheckApi(::CreatePipe(&hPipeInRead, &hPipeInWrite, nullptr, in.size() + 1));
	CheckApi(::SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	DWORD dwWritten = 0;
	CheckApi(::WriteFile(hPipeInWrite, in.c_str(), in.size(), &dwWritten, nullptr));

	// Pipe for read stdout
	auto_close<HANDLE> hPipeOutRead, hPipeOutWrite;
	CheckApi(::CreatePipe(&hPipeOutRead, &hPipeOutWrite, nullptr, 0));
	CheckApi(::SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	// fork process
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.hStdInput = hPipeInRead;
	si.hStdOutput = hPipeOutWrite;
	si.hStdError = hPipeOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//	si.lpDesktop = (PWSTR)L"hiddendesk";

	//	WinPolicy::PrivEnable("SeLoadDriverPrivilege");
	//	WinPolicy::PrivEnable(SE_INCREASE_QUOTA_NAME);
	//	WinPolicy::PrivEnable(SE_ASSIGNPRIMARYTOKEN_NAME);
	//	WinPolicy::PrivEnable(SE_RESTORE_NAME);
	//	WinPolicy::PrivEnable(SE_BACKUP_NAME);
	HANDLE hToken;
	CheckApi(::LogonUserW((PWSTR)user.c_str(), NULL, (PWSTR)pass.c_str(),
			LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, &hToken));
	//	LPVOID lpEnvironment;
	//	PROFILEINFOW pinfo;
	//	::CreateEnvironmentBlock(&lpEnvironment, hToken, false);
	//	::LoadUserProfileW(hToken, &pinfo);
	AutoUTF app = Validate(cmd);
	CheckApi(::CreateProcessAsUserW(hToken, nullptr, (PWSTR)app.c_str(), nullptr, nullptr, true, CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	hPipeInRead.close();
	hPipeInWrite.close();
	hPipeOutWrite.close();
	::CloseHandle(pi.hThread);

	auto_close<HANDLE> hProc(pi.hProcess);
	DWORD timeout = TIMEOUT;
	CHAR buf[1024 * 1024];
	DWORD dwRead;
	DWORD dwAvail = 0;
	bool childTerminate = false;
	out.clear();
	while (!childTerminate && timeout > 0) {
		if (::WaitForSingleObject(hProc, TIMEOUT_DX) == WAIT_OBJECT_0) {
			childTerminate = true;
			::GetExitCodeProcess(hProc, &Result);
		} else {
			timeout -= TIMEOUT_DX;
		}
		while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
			while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
				WinMem::Zero(buf, sizeof(buf));
				::ReadFile(hPipeOutRead, buf, sizeof(buf), &dwRead, nullptr);
				out += buf;
			}
			::Sleep(TIMEOUT_DX / 20);
		}
	}
	if (timeout <= 0) {
		throw ApiError(WAIT_TIMEOUT);
	}
	//	::UnloadUserProfile(hToken, pinfo.hProfile);
	//	::DestroyEnvironmentBlock(lpEnvironment);
	return Result;
}

HANDLE Exec::Logon(const AutoUTF &name, const AutoUTF &pass, DWORD type, const AutoUTF &dom) {
	HANDLE hToken = nullptr;
	CheckApi(::LogonUserW((PWSTR)name.c_str(), (PWSTR)dom.c_str(), (PWSTR)pass.c_str(), type, LOGON32_PROVIDER_DEFAULT, &hToken));
	return hToken;
}

void Exec::Impersonate(HANDLE hToken) {
	CheckApi(::ImpersonateLoggedOnUser(hToken));
}

HANDLE Exec::Impersonate(const AutoUTF &name, const AutoUTF &pass, DWORD type, const AutoUTF &dom) {
	HANDLE hToken = Logon(name, pass, type, dom);
	Impersonate(hToken);
	return hToken;
}

///========================================================================================== WinJob
DWORD WinJob::TIMEOUT_DX = 200;

WinJob::~WinJob() {
	::CloseHandle(m_job);
}

WinJob::WinJob() {
	m_job = ::CreateJobObject(nullptr, nullptr);
	CheckApi(m_job != nullptr);
}

WinJob::WinJob(const AutoUTF &name) {
	m_job = ::CreateJobObject(nullptr, name.c_str());
	CheckApi(m_job != nullptr);
}

void WinJob::SetTimeLimit(size_t seconds) {
	JOBOBJECT_BASIC_LIMIT_INFORMATION jobli = {{{0}}}; // funny syntax to suppress warnings
	jobli.PerJobUserTimeLimit.QuadPart = seconds * 10000;
	CheckApi(::SetInformationJobObject(m_job, JobObjectBasicLimitInformation, &jobli, sizeof(jobli)));
}

void WinJob::SetUiLimit() {
	JOBOBJECT_BASIC_UI_RESTRICTIONS jobuir = {0};
	jobuir.UIRestrictionsClass = JOB_OBJECT_UILIMIT_NONE;
	jobuir.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_EXITWINDOWS; // не имеет права останавливать систему
	jobuir.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_HANDLES; // не имеет права обращаться к USER-объектам (например, к другим окнам).
	CheckApi(::SetInformationJobObject(m_job, JobObjectBasicUIRestrictions, &jobuir, sizeof(jobuir)));
}

void WinJob::AddProcess(HANDLE hProc) {
	CheckApi(::AssignProcessToJobObject(m_job, hProc));
}

void WinJob::RunAsUser(const AutoUTF &cmd, HANDLE hToken) {
	AutoUTF app = Validate(cmd);

	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);

	CheckApi(::CreateProcessAsUserW(hToken, nullptr, (PWSTR)app.c_str(), nullptr, nullptr, false, CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	AddProcess(pi.hProcess);
	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);
}

int WinJob::RunAsUser(const AutoUTF &cmd, astring &out, const astring &in, HANDLE hToken) {
	DWORD Result = 0;

	// Pipe for write stdin
	auto_close<HANDLE> hPipeInRead, hPipeInWrite;
	CheckApi(::CreatePipe(&hPipeInRead, &hPipeInWrite, nullptr, in.size() + 1));
	CheckApi(::SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	DWORD dwWritten = 0;
	CheckApi(::WriteFile(hPipeInWrite, in.c_str(), in.size(), &dwWritten, nullptr));

	// Pipe for read stdout
	auto_close<HANDLE> hPipeOutRead, hPipeOutWrite;
	CheckApi(::CreatePipe(&hPipeOutRead, &hPipeOutWrite, nullptr, 0));
	CheckApi(::SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

	// fork process
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOW si = {0};
	si.cb = sizeof(si);
	si.hStdInput = hPipeInRead;
	si.hStdOutput = hPipeOutWrite;
	si.hStdError = hPipeOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	AutoUTF app = Validate(cmd);
	CheckApi(::CreateProcessAsUserW(hToken, nullptr, (PWSTR)app.c_str(), nullptr, nullptr, true, CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &si, &pi));
	hPipeInRead.close();
	hPipeInWrite.close();
	hPipeOutWrite.close();
	AddProcess(pi.hProcess);
	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);

	auto_close<HANDLE> hProc(pi.hProcess);
	CHAR buf[1024 * 1024];
	DWORD dwRead;
	DWORD dwAvail = 0;
	bool childTerminate = false;
	out.clear();
	while (!childTerminate) {
		if (::WaitForSingleObject(hProc, TIMEOUT_DX) == WAIT_OBJECT_0) {
			childTerminate = true;
			::GetExitCodeProcess(hProc, &Result);
		}
		while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
			while (::PeekNamedPipe(hPipeOutRead, nullptr, 0, nullptr, &dwAvail, nullptr) && dwAvail) {
				WinMem::Zero(buf, sizeof(buf));
				::ReadFile(hPipeOutRead, buf, sizeof(buf), &dwRead, nullptr);
				out += buf;
			}
			::Sleep(TIMEOUT_DX / 20);
		}
	}
	return Result;
}
