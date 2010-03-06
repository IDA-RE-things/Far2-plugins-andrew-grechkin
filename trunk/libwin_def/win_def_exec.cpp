#include "win_def.h"

///============================================================================================ Exec
DWORD					EXEC_TIMEOUT = 20000;
DWORD					EXEC_TIMEOUT_DX = 200;
bool					Exec(const CStrW &cmd) {
	HRESULT	Result = 2;
	PROCESS_INFORMATION	pi = {0};
	STARTUPINFOW		si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	CStrW	app = Validate(cmd);
	if (::CreateProcessW(NULL, app.buffer(), NULL, NULL, false, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi)) {
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
	} else {
		Result = ::GetLastError();
	}
	return	Result == NO_ERROR;
}
int						Exec(const CStrW &cmd, CStrA &out) {
	DWORD	Result = 0;
	// Pipe for read stdout
	HANDLE	hPipeOutRead, hPipeOutWrite;
	if (::CreatePipe(&hPipeOutRead, &hPipeOutWrite, NULL, 0)) {
		::SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	} else {
		return ::GetLastError();
	}

	// fork process
	PROCESS_INFORMATION	pi = {0};
	STARTUPINFOW		si = {0};
	si.cb = sizeof(si);
	si.hStdOutput = hPipeOutWrite;
	si.hStdError = hPipeOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	CStrW	app = Validate(cmd);
	if (::CreateProcessW(NULL, (PWSTR)app.c_str(), NULL, NULL, true, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi)) {
		::CloseHandle(hPipeOutWrite);
		::CloseHandle(pi.hThread);
		DWORD	timeout = EXEC_TIMEOUT;
		out.clear();
		CStrA	buf(1024 * 1024);
		DWORD	dwRead;
		DWORD	dwAvail = 0;
		bool	childTerminate = false;
		while (!childTerminate && timeout > 0) {
			if (::WaitForSingleObject(pi.hProcess, EXEC_TIMEOUT_DX) == WAIT_OBJECT_0) {
				childTerminate = true;
				::GetExitCodeProcess(pi.hProcess, &Result);
			} else {
				timeout -= EXEC_TIMEOUT_DX;
			}
			while (::PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &dwAvail, NULL) && dwAvail) {
				while (::PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &dwAvail, NULL) && dwAvail) {
					::ReadFile(hPipeOutRead, (PSTR)buf.c_str(), buf.capacity(), &dwRead, NULL);
					out += buf.c_str();
					buf.clear();
				}
				::Sleep(EXEC_TIMEOUT_DX / 20);
			}
		}
		if (timeout <= 0) {
			out = CStrA("[ERROR] Wait timeout. ") + out;
			Result = WAIT_TIMEOUT;
		}
		::CloseHandle(pi.hProcess);
	} else {
		out = CStrA("Can`t start: ") + cmd.utf8() + " [" + CStrW::err().utf8() + "]";
		::CloseHandle(hPipeOutWrite);
		Result = ERROR_BAD_COMMAND;
	}
	::CloseHandle(hPipeOutRead);
	return	Result;
}
int						Exec(const CStrW &cmd, CStrA &out, const CStrA &in) {
	DWORD	Result = 0;

	// Pipe for write stdin
	HANDLE	hPipeInRead, hPipeInWrite;
	if (::CreatePipe(&hPipeInRead, &hPipeInWrite, NULL, in.size() + 1)) {
		::SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		DWORD	dwWritten = 0;
		if (!::WriteFile(hPipeInWrite, in.c_str(), in.size(), &dwWritten, NULL))
			return	::GetLastError();
	} else
		return	::GetLastError();

	// Pipe for read stdout
	HANDLE	hPipeOutRead, hPipeOutWrite;
	if (::CreatePipe(&hPipeOutRead, &hPipeOutWrite, NULL, 1024 * 1024)) {
		::SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	} else
		return	::GetLastError();

	// fork process
	PROCESS_INFORMATION	pi = {0};
	STARTUPINFOW		si = {0};
	si.cb = sizeof(si);
	si.hStdInput = hPipeInRead;
	si.hStdOutput = hPipeOutWrite;
	si.hStdError = hPipeOutWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	CStrW	app = Validate(cmd);
	if (::CreateProcessW(NULL, (PWSTR)app.c_str(), NULL, NULL, true, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi)) {
		::CloseHandle(hPipeInRead);
		::CloseHandle(hPipeInWrite);
		::CloseHandle(hPipeOutWrite);
		::CloseHandle(pi.hThread);
		DWORD	timeout = EXEC_TIMEOUT;
		out.clear();
		CStrA	buf(1024 * 1024);
		DWORD	dwRead;
		DWORD	dwAvail = 0;
		bool	childTerminate = false;
		while (!childTerminate && timeout > 0) {
			if (::WaitForSingleObject(pi.hProcess, EXEC_TIMEOUT_DX) == WAIT_OBJECT_0) {
				childTerminate = true;
				::GetExitCodeProcess(pi.hProcess, &Result);
			} else {
				timeout -= EXEC_TIMEOUT_DX;
			}
			while (::PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &dwAvail, NULL) && dwAvail) {
				while (::PeekNamedPipe(hPipeOutRead, NULL, 0, NULL, &dwAvail, NULL) && dwAvail) {
					::ReadFile(hPipeOutRead, (PSTR)buf.c_str(), buf.capacity(), &dwRead, NULL);
					out += buf.c_str();
					buf.clear();
				}
				::Sleep(EXEC_TIMEOUT_DX / 20);
			}
		}
		if (timeout <= 0) {
			out = CStrA("[ERROR] Wait timeout. ") + out;
			Result = WAIT_TIMEOUT;
		}
		::CloseHandle(pi.hProcess);
	} else {
		out = CStrA("Can`t start: ") + cmd.utf8() + " [" + CStrW::err().utf8() + "]";
		::CloseHandle(hPipeInRead);
		::CloseHandle(hPipeInWrite);
		::CloseHandle(hPipeOutWrite);
		Result = ERROR_BAD_COMMAND;
	}
	::CloseHandle(hPipeOutRead);
	return	Result;
}
int						ExecWait(const CStrW &cmd, DWORD wait) {
	DWORD	Result = 0;

	// fork process
	PROCESS_INFORMATION	pi = {0};
	STARTUPINFOW		si = {0};
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	CStrW	app = Validate(cmd);
	if (::CreateProcessW(NULL, (PWSTR)app.c_str(), NULL, NULL, true, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi)) {
		::CloseHandle(pi.hThread);
		if (::WaitForSingleObject(pi.hProcess, wait) == WAIT_OBJECT_0) {
			::GetExitCodeProcess(pi.hProcess, &Result);
		} else {
			Result = WAIT_TIMEOUT;
		}
		::CloseHandle(pi.hProcess);
	} else {
		Result = ERROR_BAD_COMMAND;
	}
	return	Result;
}
