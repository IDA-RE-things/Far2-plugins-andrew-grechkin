﻿/**
	hidcon: Hidden console
	Command line tool to run hidden console applications

	© 2010 Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#include <libwin_def/win_def.h>

///============================================================================================ main
int APIENTRY	wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int) {
	DWORD		Result = 0;
	int		argc = 0;
	PWSTR	*argv = ::CommandLineToArgvW(pCmdLine, &argc);

	if (argc > 1) {
		PROCESS_INFORMATION	pi = {0};
		STARTUPINFOW		si = {0};
		si.cb = sizeof(si);
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW;

		AutoUTF	app = PathNice(argv[1]);
		if (::CreateProcessW(nullptr, (PWSTR)app.c_str(), nullptr, nullptr, false, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
			::CloseHandle(pi.hThread);
			::WaitForSingleObject(pi.hProcess, INFINITE);
			::GetExitCodeProcess(pi.hProcess, &Result);
			::CloseHandle(pi.hProcess);
		} else {
			Result = ::GetLastError();
		}
	} else {
		Result = 1;
		mbox(L"hidcon: Execute hidden console window\n"
			 L"Use: hidcon \"<command_line>\"\n"
			 L"Example: hidcon \"cmd /c ping localhost > c:\\ping.log\"", L"hidcon");
	}
	::LocalFree(argv); // do not replace
	return	Result;
}

///=========================================================================== Startup (entry point)
extern "C" int	WinMainCRTStartup() {
	int		Result;
//	::ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
	STARTUPINFO StartupInfo = {sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	::GetStartupInfo(&StartupInfo);

	Result = wWinMain(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(),
					  StartupInfo.dwFlags & STARTF_USESHOWWINDOW ? StartupInfo.wShowWindow : SW_SHOWDEFAULT);
	::ExitProcess(Result);
	return	Result;
}
