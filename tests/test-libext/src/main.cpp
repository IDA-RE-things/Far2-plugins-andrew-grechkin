#include <libext/exception.hpp>
#include <libext/dacl.hpp>
#include <libext/sd.hpp>
#include <libbase/logger.hpp>

Ext::WinSD init_sd(bool allow_network) {
	LogTrace();
	Ext::Sid accessSid(allow_network ? WinWorldSid : WinLocalSid);
	Ext::Sid systemSid(WinLocalSystemSid);

	Ext::ExpAccessArray eaa(2);
	new (&eaa[0]) Ext::ExpAccess(accessSid, FILE_ALL_ACCESS, SET_ACCESS);
	new (&eaa[1]) Ext::ExpAccess(systemSid, FILE_ALL_ACCESS, SET_ACCESS);

	return Ext::WinSD((PSID)accessSid, (PSID)accessSid, &eaa, nullptr);
}

int main() try {
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_color_mode(true);
	Base::Logger::set_level(Base::Logger::LVL_TRACE);
	LogTrace();

	Ext::Sid accessSid(true ? WinWorldSid : WinLocalSid);
	Ext::Sid systemSid(WinLocalSystemSid);

	LogTrace();
	Ext::ExpAccessArray eaa(2);
	new (&eaa[0]) Ext::ExpAccess(accessSid, FILE_ALL_ACCESS, SET_ACCESS);
	new (&eaa[1]) Ext::ExpAccess(systemSid, FILE_ALL_ACCESS, SET_ACCESS);

	LogTrace();
	Ext::WinSD sd(init_sd(true));

	LogTrace();
	Ext::WinSD::parse(sd);
	Ext::WinDacl::parse(sd.get_dacl());
	Ext::WinDacl::parse(sd.get_sacl());
	LogTrace();

	return 0;
} catch (Ext::AbstractError & e) {
	LogError(L"exception cought: %s, %s", e.what().c_str(), e.where());
	return e.code();
} catch (std::exception & e) {
	LogError(L"std::exception [%S]:\n", typeid(e).name());
	LogError(L"What: %S\n", e.what());
	return 1;
}
