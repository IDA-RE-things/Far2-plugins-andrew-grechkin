/**
	hidcon: Hidden console
	Command line tool to run hidden console applications

	© 2010  Andrew Grechkin
	http://code.google.com/p/andrew-grechkin

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

/// http://www.codesoil.net/2007/08/16/big-problem-access-denied-when-using-qwinstaexe-or-wtsopenserver-api/

#include "win_def.h"
#include "win_tsmgr.h"

#include <wchar.h>

///========================================================================================== define

enum	Action {
	acNone,
	acPrintHelp,
	acPrint,
	acLogoff,
	acDisconnect,
};

/// ============================================================================================ var
PCWSTR			sUser		= L"";
PCWSTR			sDom		= L"";
PCWSTR			sSess		= L"";
PCWSTR			sTitl		= L"Title";
PCWSTR			sMess		= L"";
DWORD			sId			= 65536;
Action			aAct		= acPrint;
bool			bPrintEmpty	= true;
bool			bMessage	= false;

/// ================================================================================================
void			PrintHelp() {
	wprintf(L"Terminal sessions manage (Works on XP or higher)\n(c)Grechkin Andrew, Russia, 2009\n");
	wprintf(L"\nUsage:\n");
	wprintf(L"\tts-manage [/u \"user_name\" | /s \"session\" | /i id] [/r \"\\\\server\"] [/l] [/d]\n");
	wprintf(L"\tts-manage [/p | /e] [/r \"\\\\server\"]\n");
	wprintf(L"\tts-manage [/?]\n");
	wprintf(L"Switches:\n");
	wprintf(L"\t/? - print this help\n");
	wprintf(L"\t/p - print sessions\n");
	wprintf(L"\t/e - print sessions include empty\n");
	wprintf(L"\t/l - logoff session\n");
	wprintf(L"\t/d - disconnect session\n");
	wprintf(L"\t/m \"message\"\n");
	wprintf(L"\t/t \"title\"\n");
	wprintf(L"\t/s \"session\" - session name\n");
	wprintf(L"\t/u \"user_name\" - user name\n");
	wprintf(L"\t/i id - session id\n");
	wprintf(L"\t/r \"server\" - connect to another PC\n");
	wprintf(L"Examples:\n");
	wprintf(L"\tLogoff console session:\t\t\tts-manage /s \"Console\" /l\n");
	wprintf(L"\tDisconnect user Guest on \\\\comp123:\tts-manage /u \"Guest\" /r \"\\\\comp123\" /d\n");
	wprintf(L"\tPrint sessions on \\\\mch-pc:\t\tts-manage /p /r \"\\\\mch-pc\"\n");
	wprintf(L"Remark:\n");
	wprintf(L"\tTo operate with remote sessions you must set on remote system and reboot:\n");
	wprintf(L"\t[HKLM\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server]\n");
	wprintf(L"\t\"AllowRemoteRPC\"=dword:1\n");
}
void			PrintSessions(const WinTS &sesns) {
	for (size_t i = 0; i < sesns.size(); ++i) {
		if (!bPrintEmpty && (sesns[i].state == WTSListen || WinStr::Eqi(sesns[i].user, L"")))
			continue;
		wprintf(L"id: %-5u Session: %-15s User: %-15s State: %-15s\n", sesns[i].id, sesns[i].sess, sesns[i].user, sesns[i].GetState().c_str());
	}
}

void			ParseCommandLine(size_t argc, PWSTR argv[]) {
	for (size_t i = 1; i < argc; ++i) {
		if (WinStr::Eqi(argv[i], L"/u") && i < (argc - 1)) {
			sUser = argv[i + 1];
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/s") && i < (argc - 1)) {
			sSess = argv[i + 1];
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/i") && i < (argc - 1)) {
			sId = WinStr::AsULong(argv[i + 1]);
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/r") && i < (argc - 1)) {
			sDom = argv[i + 1];
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/m") && i < (argc - 1)) {
			sMess = argv[i + 1];
			bMessage = true;
			if (aAct == acPrint)
				aAct = acNone;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/t") && i < (argc - 1)) {
			sTitl = argv[i + 1];
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/l")) {
			aAct = acLogoff;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/d")) {
			aAct = acDisconnect;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/p")) {
			aAct = acPrint;
			bPrintEmpty = false;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/e")) {
			aAct = acPrint;
			bPrintEmpty = true;
			continue;
		}
		if (WinStr::Eqi(argv[i], L"/?")) {
			aAct = acPrintHelp;
			break;
		}
	}
}

///============================================================================================ main
int				main() {
	PCWSTR	cl = ::GetCommandLineW();
	int		argc = 0;
	PWSTR	*argv = ::CommandLineToArgvW(cl, &argc);
	ParseCommandLine(argc, argv);
	if (aAct == acPrintHelp) {
		PrintHelp();
	} else {
		WinTS	sesns(sDom);
		if (sesns.IsOK()) {
			if (aAct == acPrint) {
				PrintSessions(sesns);
			} else {
				int		ind = -1;
				if (sId != 65536) {
					ind = sesns.FindId(sId);
				} else if (!WinStr::Eqi(sSess, L"")) {
					ind = sesns.FindSess(sSess);
				} else if (!WinStr::Eqi(sUser, L"")) {
					ind = sesns.FindUser(sUser);
				}
				if (ind != -1) {
					if (bMessage) {
						WinTSession::Message(sesns[ind].id, sTitl, sMess, 60, true, sDom);
					}
					switch (aAct) {
						case acDisconnect:
							WinTSession::Disconnect(sesns[ind].id, sDom);
							break;
						case acLogoff:
							WinTSession::LogOff(sesns[ind].id, sDom);
							break;
						default:
							break;
					}
				}
			}
		} else {
			wprintf(L"Error: %u\n", sesns.err());
			CStrW::err(sesns.err()).cout();
		}
	}
	::LocalFree(argv); // do not replace
	return	0;
}
