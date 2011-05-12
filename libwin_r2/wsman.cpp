#include <libwin_net/win_net.h>
#include <libwin_net/exception.h>

#include "wsman.hpp"

struct WSman: private Uncopyable {
	~WSman() {
		::WSManDeinitialize(m_hndl, 0);
	}

	static WSman &instance() {
		static WSman com;
		return com;
	}

	operator WSMAN_API_HANDLE() const {
		return m_hndl;
	}

private:
	WSman() {
		CheckApiError(::WSManInitialize(0, &m_hndl));
	}

	WSMAN_API_HANDLE m_hndl;
};

///=================================================================================== WinRS_Session
WinRS_Session::~WinRS_Session() {
	::WSManCloseSession(m_session, 0);
}

WinRS_Session::WinRS_Session(PCWSTR host, PCWSTR user, PCWSTR pass, DWORD authenticationMechanism):
	m_session(nullptr) {
	// Create a session which can be used to perform subsequent operations
	WSMAN_AUTHENTICATION_CREDENTIALS serverAuthenticationCredentials, *ac = &serverAuthenticationCredentials;
	if (user && pass) {
		serverAuthenticationCredentials.authenticationMechanism = authenticationMechanism;
		serverAuthenticationCredentials.userAccount.username = user;
		serverAuthenticationCredentials.userAccount.password = pass;
	} else {
		ac = nullptr;
	}

	CheckApiError(::WSManCreateSession(WSman::instance(), host, 0, ac, nullptr, &m_session));

	// Repeat the call to set any desired session options
	WSManSessionOption option = WSMAN_OPTION_DEFAULT_OPERATION_TIMEOUTMS;
	WSMAN_DATA data;
	data.type = WSMAN_DATA_TYPE_DWORD;
	data.number = 60000;
	CheckApiError(::WSManSetSessionOption(m_session, option, &data));

//	option = WSMAN_OPTION_UTF16;
//	data.type = WSMAN_DATA_TYPE_DWORD;
//	data.number = 1;
//	CheckApiError(::WSManSetSessionOption(m_session, option, &data));
}

///===================================================================================== WinRS_Shell
WinRS_Shell::~WinRS_Shell() {
	::WSManCloseShell(m_shell, 0, &m_Complete);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	if (NO_ERROR != m_CompleteErrorCode) {
		wprintf(L"WSManCloseShell failed: %d\n", m_CompleteErrorCode);
	}
	::CloseHandle(m_CompleteEvent);
	::CloseHandle(m_ReceiveEvent);
}

WinRS_Shell::WinRS_Shell(const WinRS_Session &session, PCWSTR resourceUri):
	m_shell(nullptr),
	m_command(nullptr),
	m_CompleteEvent(nullptr),
	m_ReceiveEvent(nullptr),
	m_CompleteErrorCode(NO_ERROR),
	m_ReceiveErrorCode(NO_ERROR) {

	// Prepare Async calls
	CheckApi(m_CompleteEvent = ::CreateEventW(0, false, false, nullptr));
	m_Complete.operationContext = this;
	m_Complete.completionFunction = &CompleteCallback;

	CheckApi(m_ReceiveEvent = ::CreateEventW(0, false, false, nullptr));
	m_Receive.operationContext = this;
	m_Receive.completionFunction = &ReceiveCallback;

//	WSMAN_OPTION oop[] = {
//		{L"protocolversion", L"2.0", true},
//	};
//	WSMAN_OPTION_SET opts;
//	opts.optionsCount = 1;
//	opts.optionsMustUnderstand = true;
//	opts.options = oop;

	::WSManCreateShell(session, 0, resourceUri, nullptr, nullptr, nullptr, &m_Complete, &m_shell);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	CheckApiError(m_CompleteErrorCode);
}

void WinRS_Shell::Execute(PCWSTR commandLine, PCWSTR argsLine) {
	WSMAN_COMMAND_ARG_SET targs = {0}, *args = &targs;
	if (argsLine) {
		targs.argsCount = 1;
		targs.args = (const WCHAR**)&argsLine;
	} else {
		args = nullptr;
	}
	::WSManRunShellCommand(m_shell, 0, commandLine, args, nullptr, &m_Complete, &m_command);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	CheckApiError(m_CompleteErrorCode);

	Send("", 1);

	WSMAN_OPERATION_HANDLE receiveOperation = nullptr;
	::WSManReceiveShellOutput(m_shell, m_command, 0, nullptr, &m_Receive, &receiveOperation);
	::WaitForSingleObject(m_ReceiveEvent, INFINITE);
	CheckApiError(m_ReceiveErrorCode);
	CheckApiError(::WSManCloseOperation(receiveOperation, 0));

	::WSManCloseCommand(m_command, 0, &m_Complete);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	CheckApiError(m_CompleteErrorCode);
}

void WinRS_Shell::Execute(PCWSTR commandLine, PSTR sendData, DWORD count) {
	::WSManRunShellCommand(m_shell, 0, commandLine, nullptr, nullptr, &m_Complete, &m_command);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	CheckApiError(m_CompleteErrorCode);

	WSMAN_OPERATION_HANDLE receiveOperation = nullptr;
	::WSManReceiveShellOutput(m_shell, m_command, 0, nullptr, &m_Receive, &receiveOperation);

	if (count) {
		for (DWORD i = 1; i <= count; ++i) {
			// last send operation should indicate end of stream
			if (!Send(sendData, (i == count))) {
				wprintf(L"Send %d failed.\n", i);
			}
		}
	}

	::WaitForSingleObject(m_ReceiveEvent, INFINITE);
	CheckApiError(m_ReceiveErrorCode);
	CheckApiError(::WSManCloseOperation(receiveOperation, 0));

	::WSManCloseCommand(m_command, 0, &m_Complete);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	CheckApiError(m_CompleteErrorCode);
}

bool WinRS_Shell::Send(PCSTR sendData, bool endOfStream) {
	WSMAN_OPERATION_HANDLE sendOperation = nullptr;
	WSMAN_DATA streamData;
	ZeroMemory(&streamData, sizeof(streamData));
	streamData.type = WSMAN_DATA_TYPE_BINARY;
	if (sendData == nullptr || strlen(sendData) == 0) {
		streamData.binaryData.dataLength = 0;
		streamData.binaryData.data = nullptr;
	} else {
		streamData.binaryData.dataLength = (strlen(sendData)) * sizeof(CHAR);
		streamData.binaryData.data = (BYTE *)sendData;
	}
	::WSManSendShellInput(m_shell, m_command, 0, WSMAN_STREAM_ID_STDIN, &streamData, endOfStream, &m_Complete, &sendOperation);
	::WaitForSingleObject(m_CompleteEvent, INFINITE);
	if (NO_ERROR != m_CompleteErrorCode) {
		wprintf(L"WSManSendShellInput failed: %d\n", m_CompleteErrorCode);
		return false;
	}
	CheckApiError(::WSManCloseOperation(sendOperation, 0));
	if (NO_ERROR != m_CompleteErrorCode) {
		wprintf(L"WSManCloseOperation failed: %d\n", m_CompleteErrorCode);
		return false;
	}

	return true;
}

// Complete async callback
void CALLBACK WinRS_Shell::CompleteCallback(PVOID operationContext,
                         DWORD flags, WSMAN_ERROR *error, WSMAN_SHELL_HANDLE shell,
                         WSMAN_COMMAND_HANDLE command, WSMAN_OPERATION_HANDLE operationHandle,
                         WSMAN_RECEIVE_DATA_RESULT *data) {
	if (operationContext) {
		WinRS_Shell *context = reinterpret_cast<WinRS_Shell *>(operationContext);
		context->m_CompleteCallback(flags, error, shell, command, operationHandle, data);
	}
}

void CALLBACK WinRS_Shell::m_CompleteCallback(DWORD /*flags*/, WSMAN_ERROR *error, WSMAN_SHELL_HANDLE /*shell*/,
                                              WSMAN_COMMAND_HANDLE /*command*/, WSMAN_OPERATION_HANDLE /*operationHandle*/,
                                              WSMAN_RECEIVE_DATA_RESULT */*data*/) {
	if (error && 0 != error->code) {
		m_CompleteErrorCode = error->code;
		// NOTE: if the errorDetail needs to be used outside of the callback,
		// then need to allocate memory, copy the content to that memory
		// as error->errorDetail itself is owned by WSMan client stack and will
		// be deallocated and invalid when the callback exits
		printf(L"%s\n", error->errorDetail);
	}
	::SetEvent(m_CompleteEvent);
}

// Receive async callback
void CALLBACK WinRS_Shell::ReceiveCallback(PVOID operationContext,
                         DWORD flags, WSMAN_ERROR *error, WSMAN_SHELL_HANDLE shell,
                         WSMAN_COMMAND_HANDLE command, WSMAN_OPERATION_HANDLE operationHandle,
                         WSMAN_RECEIVE_DATA_RESULT *data) {
	if (operationContext) {
		WinRS_Shell * context = reinterpret_cast<WinRS_Shell *>(operationContext);
		context->m_ReceiveCallback(flags, error, shell, command, operationHandle, data);
	}
}

void CALLBACK WinRS_Shell::m_ReceiveCallback(DWORD /*flags*/, WSMAN_ERROR *error, WSMAN_SHELL_HANDLE /*shell*/,
                         WSMAN_COMMAND_HANDLE /*command*/, WSMAN_OPERATION_HANDLE /*operationHandle*/,
                         WSMAN_RECEIVE_DATA_RESULT *data)
{
	if (error && 0 != error->code) {
		m_ReceiveErrorCode = error->code;
		// NOTE: if the errorDetail needs to be used outside of the callback,
		// then need to allocate memory, copy the content to that memory
		// as error->errorDetail itself is owned by WSMan client stack and will
		// be deallocated and invalid when the callback exits
		printf(L"%s\n", error->errorDetail);
	}

//	printf(L"error: 0x%x\n", error);
//	if (error) {
//		printf(L"error->code: %d\n", error->code);
//	}
//
//	printf(L"data: 0x%x\n", data);
//	if (data) {
//		printf(L"data->commandState: %s\n", data->commandState);
//	}

	// Output the received data to the console
	if (data && data->streamData.type == WSMAN_DATA_TYPE_BINARY && data->streamData.binaryData.dataLength) {
		HANDLE hFile = ((0 == _wcsicmp(data->streamId, WSMAN_STREAM_ID_STDERR)) ?
		                    ::GetStdHandle(STD_ERROR_HANDLE) : ::GetStdHandle(STD_OUTPUT_HANDLE));

		DWORD written = 0;
		::WriteFile(hFile, data->streamData.binaryData.data, data->streamData.binaryData.dataLength,
		            &written, nullptr);
	}

	// for WSManReceiveShellOutput, needs to wait for state to be done before signalliing the end of the operation
	if ((error && 0 != error->code) ||
		(data && data->commandState && wcscmp(data->commandState, WSMAN_COMMAND_STATE_DONE) == 0)) {
		::SetEvent(m_ReceiveEvent);
	}
}
