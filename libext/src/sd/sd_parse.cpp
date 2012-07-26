#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/dacl.hpp>
#include <libbase/bit.hpp>
#include <libbase/bit_str.hpp>
#include <libbase/str.hpp>

namespace Ext {

	ustring WinSD::Parse(PSECURITY_DESCRIPTOR sd) {
		using namespace Base;
		ustring Result;
		WORD ctrl = get_control(sd);
		Result += L"Security descriptor:\n";
		Result += ustring(L"SDDL: ") + WinSD::as_sddl(sd, DACL_SECURITY_INFORMATION);

		Result += ustring(L"\nSize: ") + Base::as_str(size(sd));
		Result += ustring(L"\tOwner: ") + Sid::get_name(get_owner(sd));
		Result += ustring(L"\tGroup: ") + Sid::get_name(get_group(sd));
		Result += ustring(L"\nControl: 0x") + Base::as_str(ctrl, 16) + L" (" + BitMask<WORD>::as_str_bin(ctrl) + L") [" + BitMask<WORD>::as_str_num(ctrl) + L"]\n";
		if (WinFlag::Check(ctrl, (WORD)SE_OWNER_DEFAULTED))
			Result += ustring(L"\tSE_OWNER_DEFAULTED (") + Base::as_str(SE_OWNER_DEFAULTED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_GROUP_DEFAULTED))
			Result += ustring(L"\tSE_GROUP_DEFAULTED (") + Base::as_str(SE_GROUP_DEFAULTED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_DACL_PRESENT))
			Result += ustring(L"\tSE_DACL_PRESENT (") + Base::as_str(SE_DACL_PRESENT) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SACL_DEFAULTED))
			Result += ustring(L"\tSE_DACL_DEFAULTED (") + Base::as_str(SE_SACL_DEFAULTED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SACL_PRESENT))
			Result += ustring(L"\tSE_DACL_PRESENT (") + Base::as_str(SE_SACL_PRESENT) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SACL_DEFAULTED))
			Result += ustring(L"\tSE_SACL_DEFAULTED (") + Base::as_str(SE_SACL_DEFAULTED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_DACL_AUTO_INHERIT_REQ))
			Result += ustring(L"\tSE_DACL_AUTO_INHERIT_REQ (") + Base::as_str(SE_DACL_AUTO_INHERIT_REQ) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SACL_AUTO_INHERIT_REQ))
			Result += ustring(L"\tSE_SACL_AUTO_INHERIT_REQ (") + Base::as_str(SE_SACL_AUTO_INHERIT_REQ) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_DACL_AUTO_INHERITED))
			Result += ustring(L"\tSE_DACL_AUTO_INHERITED (") + Base::as_str(SE_DACL_AUTO_INHERITED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SACL_AUTO_INHERITED))
			Result += ustring(L"\tSE_SACL_AUTO_INHERITED (") + Base::as_str(SE_SACL_AUTO_INHERITED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_DACL_PROTECTED))
			Result += ustring(L"\tSE_DACL_PROTECTED (") + Base::as_str(SE_DACL_PROTECTED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SACL_PROTECTED))
			Result += ustring(L"\tSE_SACL_PROTECTED (") + Base::as_str(SE_SACL_PROTECTED) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_SELF_RELATIVE))
			Result += ustring(L"\tSE_SELF_RELATIVE (") + Base::as_str(SE_SELF_RELATIVE) + L")\n";
		if (WinFlag::Check(ctrl, (WORD)SE_DACL_PRESENT)) {
			Result += as_str(get_dacl(sd));
		}
		return Result;
	}

}
