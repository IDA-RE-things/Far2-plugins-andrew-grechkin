#include <libext/dacl.hpp>
#include <libext/exception.hpp>
#include <libbase/logger.hpp>
#include <libbase/bit.hpp>
#include <libbase/bit_str.hpp>

namespace Ext {

	Base::NamedValues<BYTE> aceTypes[] = {
		{L"ACCESS_ALLOWED_ACE_TYPE", ACCESS_ALLOWED_ACE_TYPE},
		{L"ACCESS_DENIED_ACE_TYPE", ACCESS_DENIED_ACE_TYPE},
		{L"SYSTEM_AUDIT_ACE_TYPE", SYSTEM_AUDIT_ACE_TYPE},
		{L"ACCESS_ALLOWED_OBJECT_ACE_TYPE", ACCESS_ALLOWED_OBJECT_ACE_TYPE},
		{L"ACCESS_DENIED_OBJECT_ACE_TYPE", ACCESS_DENIED_OBJECT_ACE_TYPE},
		{L"SYSTEM_AUDIT_OBJECT_ACE_TYPE", SYSTEM_AUDIT_OBJECT_ACE_TYPE},
	};

	Base::NamedValues<ULONG> aceFlags[] = {
		{L"INHERITED_ACE", INHERITED_ACE},
		{L"CONTAINER_INHERIT_ACE", CONTAINER_INHERIT_ACE},
		{L"OBJECT_INHERIT_ACE", OBJECT_INHERIT_ACE},
		{L"INHERIT_ONLY_ACE", INHERIT_ONLY_ACE},
		{L"NO_PROPAGATE_INHERIT_ACE", NO_PROPAGATE_INHERIT_ACE},
		{L"FAILED_ACCESS_ACE_FLAG", FAILED_ACCESS_ACE_FLAG},
		{L"SUCCESSFUL_ACCESS_ACE_FLAG", SUCCESSFUL_ACCESS_ACE_FLAG}
	};

	void WinDacl::parse(PACL acl) {
#ifndef NO_LOGGER
		LogInfo(L"ACL:\n");
		if (!acl) {
			LogInfo(L"\tNULL\tAll access allowed\n");
			return;
		}

		ACL_SIZE_INFORMATION aclSize;
		WinDacl::get_info(acl, aclSize);
		if (aclSize.AceCount == 0) {
			LogInfo(L"\tEmpty\tNo access allowed\n");
			return;
		}

		LogInfo(L"\tACE count: %u \tUse: %u \tFree: %u\n", aclSize.AceCount, aclSize.AclBytesInUse, aclSize.AclBytesFree);
		for (ULONG lIndex = 0; lIndex < aclSize.AceCount; ++lIndex) {
			ACCESS_ALLOWED_ACE *pACE;
			if (!::GetAce(acl, lIndex, (PVOID*)&pACE))
				return;

			try {
				PSID pSID = PSIDFromPACE(pACE);
				LogInfo(L"ACE [%u] \tACE Name: %s (%s)\n", lIndex, Sid::get_name(pSID).c_str(), Sid::as_str(pSID).c_str());

				ULONG lIndex2 = 6;
				PCWSTR pszString = L"Unknown ACE Type";
				while (lIndex2--) {
					if (pACE->Header.AceType == aceTypes[lIndex2].value) {
						pszString = aceTypes[lIndex2].name;
						break;
					}
				}
				LogInfo(L"\tAceType: %s\n", pszString);
				LogInfo(L"\tACE Mask: %s\n", Base::BitMask<DWORD>::as_str_bin(pACE->Mask).c_str());

				LogInfo(L"\tACE Flags:\n", pszString);
				lIndex2 = 7;
				while (lIndex2--) {
					if ((pACE->Header.AceFlags & aceFlags[lIndex2].value) != 0) {
						LogInfo(L"\t\t%s\n", aceFlags[lIndex2].name);
					}
				}

			} catch (...) {
			}
		}
#else
	(void)acl;
#endif
	}

}
