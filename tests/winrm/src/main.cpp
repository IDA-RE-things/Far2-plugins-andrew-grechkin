#include <libwin_net/win_net.h>
#include <libwin_net/exception.h>
#include <libwin_def/console.h>
#include <libwin_r2/wsman.hpp>

int main() {
	try {
//		printf(L"Open session\n");
		WinRS_Session ses(L"win2008-std"); //(L"http://localhost:5985/WSMAN");
		printf(L"a1\n");
//		WinRS_Session ses; //(L"http://localhost:5985/WSMAN");
		printf(L"a2\n");
		WinRS_Shell shell(ses);//, L"http://schemas.microsoft.com/powershell/Microsoft.PowerShell");
		printf(L"a3\n");
		{
			ustring cmd(L"ping localhost");
	//		ustring cmd(L"powershell.exe");
			printf(L"Execute command: %s\n", cmd.c_str());
			shell.Execute(cmd.c_str());
			printf(L"Execute completed\n\n");
		}
		printf(L"a4\n");
		{
			ustring cmd(L"dir");
			printf(L"Execute command: %s\n", cmd.c_str());
			shell.Execute(cmd.c_str(), L"c:\temp /q");
			printf(L"Execute completed\n\n");
		}
		{
//			::Sleep(100000);
//			ustring cmd(L"PowerShell");
//			shell.Execute(cmd.c_str(), L"-Command \"&{Get-Host | ConvertTo-XML | Write-Host}\"");
			shell.Execute(L"powershell", L"-Command \"&{get-host}\"");
//			shell.Execute(cmd.c_str(), nullptr);
		}
		return 0;

		ustring cmd(L"get-host");
		printf(L"Execute command: %s\n", cmd.c_str());
//		shell.Execute(cmd.c_str());
		printf(L"Execute completed\n");

	} catch (WinError & e) {
		printf(L"\nError: %Id\n", e.code());
		printf(L"Where: %s\n", e.where().c_str());
		printf(L"What: %s\n", e.what().c_str());
	} catch (...) {
		printf(L"Error");
	}
	return 0;
}
