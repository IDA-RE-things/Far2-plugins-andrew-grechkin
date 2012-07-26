#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libbase/bit.hpp>

namespace Ext {

	ustring Mode2Sddl(const ustring & owner, const ustring & group, mode_t mode) {
		ustring Result = L"(A;OICI;FA;;;BA)";
		ustring sm;
		if ((mode & 0700) && !owner.empty()) {
			if ((mode & 0700) == 0700) {
				sm += L"FA";
			} else {
				if (Base::WinFlag::Check(mode, (mode_t)S_IRUSR))
					sm += L"FR";
				if (Base::WinFlag::Check(mode, (mode_t)S_IWUSR))
					sm += L"FWSDWDWO";
				if (Base::WinFlag::Check(mode, (mode_t)S_IXUSR))
					sm += L"FX";
			}
			Result += ustring(L"(A;OICI;") + sm + L";;;" + Sid(owner).as_str() + L")";
		}
		sm.clear();
		if ((mode & 070) && !group.empty()) {
			if ((mode & 070) == 070) {
				sm += L"FA";
			} else {
				if (Base::WinFlag::Check(mode, (mode_t)S_IRGRP))
					sm += L"FR";
				if (Base::WinFlag::Check(mode, (mode_t)S_IWGRP))
					sm += L"FWSDWDWO";
				if (Base::WinFlag::Check(mode, (mode_t)S_IXGRP))
					sm += L"FX";
			}
			Result += ustring(L"(A;OICI;") + sm + L";;;" + Sid(group).as_str() + L")";
		}
		sm.clear();
		if (mode & 07) {
			if ((mode & 07) == 07) {
				sm += L"FA";
			} else {
				if (Base::WinFlag::Check(mode, (mode_t)S_IROTH))
					sm += L"FR";
				if (Base::WinFlag::Check(mode, (mode_t)S_IWOTH))
					sm += L"FWSDWDWO";
				if (Base::WinFlag::Check(mode, (mode_t)S_IXOTH))
					sm += L"FX";
			}
			Result += ustring(L"(A;OICI;") + sm + L";;;WD)";
		}
		return Result;
	}

}
