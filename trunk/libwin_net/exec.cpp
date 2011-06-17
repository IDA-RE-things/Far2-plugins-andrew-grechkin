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
		throw WinError(WAIT_TIMEOUT);
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
		throw WinError(WAIT_TIMEOUT);
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
		throw WinError(WAIT_TIMEOUT);
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
		throw WinError(WAIT_TIMEOUT);
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


// Функция-пускатель внешних процессов
// Возвращает -1 в случае ошибки или...
/*
int Execute(const wchar_t *CmdStr, // Ком.строка для исполнения
            bool AlwaysWaitFinish, // Ждать завершение процесса?
            bool SeparateWindow,   // Выполнить в отдельном окне?
            bool DirectRun,        // Выполнять директом? (без CMD)
            bool FolderRun,        // Это фолдер?
            bool WaitForIdle,      // for list files
            bool Silent,
            bool RunAs             // elevation
            )
{
	int nResult = -1;
	string strNewCmdStr;
	string strNewCmdPar;
	PartCmdLine(CmdStr, strNewCmdStr, strNewCmdPar);

	DWORD dwAttr = apiGetFileAttributes(strNewCmdStr);

	if(RunAs)
	{
		SeparateWindow = true;
	}

	if(FolderRun)
	{
		Silent = true;
	}

	if (SeparateWindow)
	{
		if(Opt.ExecuteSilentExternal)
		{
			Silent = true;
		}
		if (strNewCmdPar.IsEmpty() && dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			ConvertNameToFull(strNewCmdStr, strNewCmdStr);
			DirectRun = true;
			FolderRun=true;
		}
	}

	string strComspec;
	apiGetEnvironmentVariable(L"COMSPEC", strComspec);
	if (strComspec.IsEmpty() && !DirectRun)
	{
		Message(MSG_WARNING, 1, MSG(MWarning), MSG(MComspecNotFound), MSG(MErrorCancelled), MSG(MOk));
		return -1;
	}

	DWORD dwSubSystem;
	DWORD dwError = 0;
	HANDLE hProcess = nullptr;
	LPCWSTR lpVerb = nullptr;

	if (FolderRun && DirectRun)
	{
		AddEndSlash(strNewCmdStr); // НАДА, иначе ShellExecuteEx "возьмет" BAT/CMD/пр.ересь, но не каталог
	}
	else
	{
		FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem);

		if ( dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN)
		{
			DWORD Error=0, dwSubSystem2=0;
			size_t pos;

			if (strNewCmdStr.RPos(pos,L'.'))
			{
				const wchar_t *ExtPtr=strNewCmdStr.CPtr()+pos;
				if (!(!StrCmpI(ExtPtr,L".exe") || !StrCmpI(ExtPtr,L".com") || IsBatchExtType(ExtPtr)))
				{
					lpVerb=GetShellAction(strNewCmdStr,dwSubSystem2,Error);

					if (lpVerb && Error != ERROR_NO_ASSOCIATION)
					{
						dwSubSystem=dwSubSystem2;
					}
				}

				if (dwSubSystem == IMAGE_SUBSYSTEM_UNKNOWN && !StrCmpNI(strNewCmdStr,L"ECHO.",5)) // вариант "echo."
				{
					strNewCmdStr.Replace(pos,1,L' ');
					PartCmdLine(strNewCmdStr,strNewCmdStr,strNewCmdPar);

					if (strNewCmdPar.IsEmpty())
						strNewCmdStr+=L'.';

					FindModule(strNewCmdStr,strNewCmdStr,dwSubSystem);
				}
			}
		}

		if (dwSubSystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			if(DirectRun && Opt.ExecuteSilentExternal)
			{
				Silent = true;
			}
			DirectRun = true;
			SeparateWindow = true;
		}
	}

	bool Visible=false;
	DWORD Size=0;
	SMALL_RECT ConsoleWindowRect;
	COORD ConsoleSize;
	int ConsoleCP = CP_OEMCP;
	int ConsoleOutputCP = CP_OEMCP;

	if(!Silent)
	{
		int X1, X2, Y1, Y2;
		CtrlObject->CmdLine->GetPosition(X1, Y1, X2, Y2);
		ProcessShowClock++;
		CtrlObject->CmdLine->ShowBackground();
		CtrlObject->CmdLine->Redraw();
		GotoXY(X2+1,Y1);
		Text(L" ");
		MoveCursor(X1,Y1);
		GetCursorType(Visible,Size);
		SetInitialCursorType();
	}

	CtrlObject->CmdLine->SetString(L"", Silent);

	if(!Silent)
	{
		// BUGBUG: если команда начинается с "@", то эта строка херит все начинания
		// TODO: здесь необходимо подставить виртуальный буфер, а потом его корректно подсунуть в ScrBuf
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	SHELLEXECUTEINFO seInfo={sizeof(seInfo)};
	string strCurDir;
	apiGetCurrentDirectory(strCurDir);
	seInfo.lpDirectory=strCurDir;
	seInfo.nShow = SW_SHOWNORMAL;

	string strFarTitle;
	if(!Silent)
	{
		if (Opt.ExecuteFullTitle)
		{
			strFarTitle += strNewCmdStr;
			if (!strNewCmdPar.IsEmpty())
			{
				strFarTitle.Append(L" ").Append(strNewCmdPar);
			}
		}
		else
		{
			strFarTitle+=CmdStr;
		}
		ConsoleTitle::SetFarTitle(strFarTitle);
	}

	string ComSpecParams(L"/C ");
	if (DirectRun)
	{
		seInfo.lpFile = strNewCmdStr;
		seInfo.lpParameters = strNewCmdPar;
		seInfo.lpVerb = (dwAttr&FILE_ATTRIBUTE_DIRECTORY)?nullptr:lpVerb?lpVerb:GetShellAction(strNewCmdStr, dwSubSystem, dwError);
	}
	else
	{
		QuoteSpace(strNewCmdStr);
		bool bDoubleQ = wcspbrk(strNewCmdStr, L"&<>()@^|=;, ") != nullptr;
		if (!strNewCmdPar.IsEmpty() || bDoubleQ)
		{
			ComSpecParams += L"\"";
		}
		ComSpecParams += strNewCmdStr;
		if (!strNewCmdPar.IsEmpty())
		{
			ComSpecParams.Append(L" ").Append(strNewCmdPar);
		}
		if (!strNewCmdPar.IsEmpty() || bDoubleQ)
		{
			ComSpecParams += L"\"";
		}

		seInfo.lpFile = strComspec;
		seInfo.lpParameters = ComSpecParams;
		seInfo.lpVerb = nullptr;
	}

	if(RunAs && RunAsSupported(seInfo.lpFile))
	{
		SetCurrentDirectory(seInfo.lpDirectory);
		seInfo.lpVerb = L"runas";
	}

	seInfo.fMask = SEE_MASK_FLAG_NO_UI|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS|(SeparateWindow?0:SEE_MASK_NO_CONSOLE);

	if (ShellExecuteEx(&seInfo))
	{
		hProcess = seInfo.hProcess;
	}
	else
	{
		dwError = GetLastError();
	}

	DWORD ErrorCode=0;
	bool ErrMsg=false;

	if (!dwError)
	{
		if (hProcess)
		{
			ScrBuf.Flush();

			if (AlwaysWaitFinish || !SeparateWindow)
			{
				if (!Opt.ConsoleDetachKey)
				{
					WaitForSingleObject(hProcess,INFINITE);
				}
				else
				{
//					$ 12.02.2001 SKV
//					  супер фитча ;)
//					  Отделение фаровской консоли от неинтерактивного процесса.
//					  Задаётся кнопкой в System/ConsoleDetachKey

					HANDLE hOutput = Console.GetOutputHandle();
					HANDLE hInput = Console.GetInputHandle();
					INPUT_RECORD ir[256];
					DWORD rd;
					int vkey=0,ctrl=0;
					TranslateKeyToVK(Opt.ConsoleDetachKey,vkey,ctrl,nullptr);
					int alt=ctrl&PKF_ALT;
					int shift=ctrl&PKF_SHIFT;
					ctrl=ctrl&PKF_CONTROL;
					bool bAlt, bShift, bCtrl;
					DWORD dwControlKeyState;

					//Тут нельзя делать WaitForMultipleObjects из за бага в Win7 при работе в телнет
					while (WaitForSingleObject(hProcess, 100) != WAIT_OBJECT_0)
					{
						if (WaitForSingleObject(hInput, 100)==WAIT_OBJECT_0 && Console.PeekInput(*ir, 256, rd) && rd)
						{
							int stop=0;

							for (DWORD i=0; i<rd; i++)
							{
								PINPUT_RECORD pir=&ir[i];

								if (pir->EventType==KEY_EVENT)
								{
									dwControlKeyState = pir->Event.KeyEvent.dwControlKeyState;
									bAlt = (dwControlKeyState & LEFT_ALT_PRESSED) || (dwControlKeyState & RIGHT_ALT_PRESSED);
									bCtrl = (dwControlKeyState & LEFT_CTRL_PRESSED) || (dwControlKeyState & RIGHT_CTRL_PRESSED);
									bShift = (dwControlKeyState & SHIFT_PRESSED)!=0;

									if (vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
									        (alt ?bAlt:!bAlt) &&
									        (ctrl ?bCtrl:!bCtrl) &&
									        (shift ?bShift:!bShift))
									{
										HICON hSmallIcon=nullptr,hLargeIcon=nullptr;
										HWND hWnd = Console.GetWindow();

										if (hWnd)
										{
											hSmallIcon = CopyIcon((HICON)SendMessage(hWnd,WM_SETICON,0,(LPARAM)0));
											hLargeIcon = CopyIcon((HICON)SendMessage(hWnd,WM_SETICON,1,(LPARAM)0));
										}

										Console.ReadInput(*ir, 256, rd);
//										  Не будем вызыват CloseConsole, потому, что она поменяет
//										  ConsoleMode на тот, что был до запуска Far'а,
//										  чего работающее приложение могло и не ожидать.
										CloseHandle(hInput);
										CloseHandle(hOutput);
										delete KeyQueue;
										KeyQueue=nullptr;
										Console.Free();
										Console.Allocate();

										if (hWnd)   // если окно имело HOTKEY, то старое должно его забыть.
											SendMessage(hWnd,WM_SETHOTKEY,0,(LPARAM)0);

										Console.SetSize(ConsoleSize);
										Console.SetWindowRect(ConsoleWindowRect);
										Console.SetSize(ConsoleSize);
										Sleep(100);
										InitConsole(0);

										hWnd = Console.GetWindow();

										if (hWnd)
										{
											if (Opt.SmallIcon)
											{
												ExtractIconEx(g_strFarModuleName,0,&hLargeIcon,&hSmallIcon,1);
											}

											if (hLargeIcon )
												SendMessage(hWnd,WM_SETICON,1,(LPARAM)hLargeIcon);

											if (hSmallIcon )
												SendMessage(hWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
										}

										stop=1;
										break;
									}
								}
							}

							if (stop)
								break;
						}
					}
				}

				if(!Silent)
				{
					bool SkipScroll = false;
					COORD Size;
					if(Console.GetSize(Size))
					{
						COORD BufferSize = {Size.X, Opt.ShowKeyBar?3:2};
						PCHAR_INFO Buffer = new CHAR_INFO[BufferSize.X * BufferSize.Y];
						COORD BufferCoord = {};
						SMALL_RECT ReadRegion = {0, Size.Y - BufferSize.Y, Size.X, Size.Y};
						if(Console.ReadOutput(*Buffer, BufferSize, BufferCoord, ReadRegion))
						{
							WORD Attributes = Buffer[BufferSize.X*BufferSize.Y-1].Attributes;
							SkipScroll = true;
							for(int i = 0; i < BufferSize.X*BufferSize.Y; i++)
							{
								if(Buffer[i].Char.UnicodeChar != L' ' || Buffer[i].Attributes != Attributes)
								{
									SkipScroll = false;
									break;
								}
							}
							delete[] Buffer;
						}
					}
					if(!SkipScroll)
					{
						Console.ScrollScreenBuffer(Opt.ShowKeyBar?2:1);
					}
				}

			}
			if(WaitForIdle)
			{
				WaitForInputIdle(hProcess, INFINITE);
			}
			CloseHandle(hProcess);
		}

		nResult = 0;
	}
	else
	{

		if (Opt.ExecuteShowErrorMessage)
		{
			ErrorCode=GetLastError();
			ErrMsg=true;
		}
		else
		{
			string strOutStr;
			strOutStr.Format(MSG(MExecuteErrorMessage),strNewCmdStr.CPtr());
			string strPtrStr=FarFormatText(strOutStr,ScrX,strPtrStr,L"\n",0);
			Console.Write(strPtrStr, static_cast<DWORD>(strPtrStr.GetLength()));
		}
	}

	if(ErrMsg)
	{
		SetLastError(ErrorCode);
		SetMessageHelp(L"ErrCannotExecute");
		string strOutStr = strNewCmdStr;
		Unquote(strOutStr);
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),strOutStr,MSG(MOk));
	}

	return nResult;
}
*/
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
