#ifndef WIN_NET_POLICY_HPP
#define WIN_NET_POLICY_HPP

#include "win_net.h"

#include <ntsecapi.h>

///======================================================================================= WinPolicy
namespace	WinPolicy {
	void		InitLsaString(LSA_UNICODE_STRING & lsaString, const ustring & in);

	LSA_HANDLE	GetPolicyHandle(const ustring & dom = ustring());

	NTSTATUS	AccountRightAdd(const ustring & name, const ustring & right, const ustring & dom = ustring());

	NTSTATUS	AccountRightDel(const ustring & name, const ustring & right, const ustring & dom = ustring());

	bool		GetTokenUser(HANDLE	hToken, ustring & name);
}

#endif
