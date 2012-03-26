#include <libwin_net/exception.h>
#include <libwin_net/auth.h>
#include <libwin_net/sid.h>
#include <libwin_net/priv.h>
#include <libwin_def/console.h>

#include <Ntdef.h>

extern "C" NTSYSAPI NTSTATUS NTAPI
NtCreateToken(PHANDLE TokenHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes,
              TOKEN_TYPE TokenType, PLUID AuthenticationId, PLARGE_INTEGER ExpirationTime,
              PTOKEN_USER TokenUser, PTOKEN_GROUPS TokenGroups, PTOKEN_PRIVILEGES TokenPrivileges,
              PTOKEN_OWNER TokenOwner, PTOKEN_PRIMARY_GROUP TokenPrimaryGroup,
              PTOKEN_DEFAULT_DACL TokenDefaultDacl, PTOKEN_SOURCE TokenSource);

extern "C" WINAPI NTSTATUS NtAllocateLocallyUniqueId(PLUID LUID);

int main() try {
//	Credentials_t creds;
//
//	for (size_t i = 0; i < creds.size(); ++i) {
//		printf(L"cred[%Id]:\n", i);
//		printf(L"\tUserName: %s\n", creds.at(i)->UserName);
//		printf(L"\tTargetAlias: %s\n", creds.at(i)->TargetAlias);
//		printf(L"\tTargetName: %s\n", creds.at(i)->TargetName);
//		printf(L"\tComment: %s\n", creds.at(i)->Comment);
//	}

	Sid user_sid(L"test_user");

	Privilege priv(SE_CREATE_TOKEN_NAME);

//	HANDLE token = nullptr;
	LUID luid;
	LARGE_INTEGER expir;


	CheckApiError(NtAllocateLocallyUniqueId(&luid));

	NtCreateToken(&token, TOKEN_ALL_ACCESS, nullptr, TokenPrimary, &luid, );




	return 0;
} catch (WinError & e) {
	return e.format_error();
} catch (std::exception & e) {
	printf("std::exception [%s]:\n", typeid(e).name());
	printf("What: %s\n", e.what());
	return 1;
}
