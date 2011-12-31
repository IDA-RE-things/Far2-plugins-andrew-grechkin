#ifndef WIN_NET_EXEC_HPP
#define WIN_NET_EXEC_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_exec
///============================================================================================ Exec
namespace Exec {
	extern DWORD	TIMEOUT;
	extern DWORD	TIMEOUT_DX;
	void	Run(const ustring &cmd);
	int		Run(const ustring &cmd, astring &out);
	int		Run(const ustring &cmd, astring &out, const astring &in);
	int		RunWait(const ustring &cmd, DWORD wait = TIMEOUT);
	void	RunAsUser(const ustring &cmd, HANDLE token);
	void	RunAsUser(const ustring &cmd, const ustring &user, const ustring &pass);
	int		RunAsUser(const ustring &cmd, astring &out, const astring &in, const ustring &user, const ustring &pass);
	HANDLE	Logon(const ustring &name, const ustring &pass, DWORD type, const ustring &dom = ustring());
	void	Impersonate(HANDLE hToken);
	HANDLE	Impersonate(const ustring &name, const ustring &pass, DWORD type = LOGON32_LOGON_BATCH, const ustring &dom = ustring());
}

///========================================================================================== WinJob
struct WinJob {
	~WinJob();
	WinJob();
	WinJob(const ustring &name);
	void	SetTimeLimit(size_t seconds);
	void	SetUiLimit();
	void	AddProcess(HANDLE hProc);
	void	RunAsUser(const ustring &cmd, HANDLE hToken);
	int		RunAsUser(const ustring &cmd, astring &out, const astring &in, HANDLE hToken);
private:
	static DWORD	TIMEOUT_DX;
	HANDLE m_job;
};

#endif
