#ifndef _WIN_R2_WSMAN_HPP_
#define _WIN_R2_WSMAN_HPP_

#include <windows.h>

#define WSMAN_API_VERSION_1_0
#include <wsman.h>

///=================================================================================== WinRS_Session
class WinRS_Session {
public:
	~WinRS_Session();

	WinRS_Session(PCWSTR host = nullptr,
	              PCWSTR username = nullptr, PCWSTR password = nullptr,
	              DWORD authenticationMechanism = WSMAN_FLAG_AUTH_KERBEROS);

	operator WSMAN_SESSION_HANDLE() const {
		return m_session;
	}

private:
	WSMAN_SESSION_HANDLE m_session;
};

///===================================================================================== WinRS_Shell
class WinRS_Shell {
public:
	~WinRS_Shell();

	WinRS_Shell(const WinRS_Session & session, PCWSTR resourceUri = WSMAN_CMDSHELL_URI);

	void Execute(PCWSTR commandLine, PCWSTR argLine = nullptr);

	void Execute(PCWSTR commandLine, PSTR sendData, DWORD size);

private:
	bool Send(PCSTR sendData, bool endOfStream);

	static void CALLBACK CompleteCallback(PVOID operationContext,
		DWORD flags, WSMAN_ERROR * error, WSMAN_SHELL_HANDLE shell,
		WSMAN_COMMAND_HANDLE command, WSMAN_OPERATION_HANDLE operationHandle,
		WSMAN_RECEIVE_DATA_RESULT * data
	);
	void CALLBACK m_CompleteCallback(
		DWORD flags, WSMAN_ERROR * error, WSMAN_SHELL_HANDLE shell,
		WSMAN_COMMAND_HANDLE command, WSMAN_OPERATION_HANDLE operationHandle,
		WSMAN_RECEIVE_DATA_RESULT * data
	);

	static void CALLBACK ReceiveCallback(PVOID operationContext,
		DWORD flags, WSMAN_ERROR * error, WSMAN_SHELL_HANDLE shell,
		WSMAN_COMMAND_HANDLE command, WSMAN_OPERATION_HANDLE operationHandle,
		WSMAN_RECEIVE_DATA_RESULT * data
	);
	void CALLBACK m_ReceiveCallback(
		DWORD flags, WSMAN_ERROR * error, WSMAN_SHELL_HANDLE shell,
		WSMAN_COMMAND_HANDLE command, WSMAN_OPERATION_HANDLE operationHandle,
		WSMAN_RECEIVE_DATA_RESULT * data
	);

private:
	WSMAN_SHELL_HANDLE m_shell;
	WSMAN_COMMAND_HANDLE m_command;

	WSMAN_SHELL_ASYNC m_Complete;
	WSMAN_SHELL_ASYNC m_Receive;
	HANDLE m_CompleteEvent;
	HANDLE m_ReceiveEvent;
	DWORD m_CompleteErrorCode;
	DWORD m_ReceiveErrorCode;
};

#endif
