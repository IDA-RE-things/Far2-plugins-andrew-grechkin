/**
	delstr: Delete strings in editor
	FAR3lua plugin

	© 2013 Andrew Grechkin

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

#include <globalinfo.hpp>
#include <farplugin.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/plugin_i.hpp>
#include <libbase/logger.hpp>

///========================================================================================== Export
/// GlobalInfo
void WINAPI GetGlobalInfoW(GlobalInfo * Info)
{
	Base::Logger::set_target(Base::Logger::get_TargetToFile(L"D:/projects/~test/delstr.log"));
	Base::Logger::set_level(Base::Logger::Level::Trace);

	LogTrace();
	LogNoise(L"==========================================================================\n");

	Far::helper_t::inst().init(new FarGlobalInfo);

	get_global_info()->GetGlobalInfoW(Info);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo * Info)
{
	LogTrace();
	get_global_info()->SetStartupInfoW(Info);
	get_global_info()->load_settings();
}

intptr_t WINAPI ConfigureW(const ConfigureInfo * Info)
{
	LogTrace();
	return get_global_info()->ConfigureW(Info);
}

/// Plugin
void WINAPI GetPluginInfoW(PluginInfo * Info)
{
	LogTrace();
	Far::helper_t::inst().get_plugin()->GetPluginInfoW(Info);
}

HANDLE WINAPI OpenW(const OpenInfo * Info)
{
	LogTrace();
	return Far::helper_t::inst().get_plugin()->OpenW(Info);
}

void WINAPI ExitFARW(const ExitInfo * Info)
{
	LogTrace();
	Far::helper_t::inst().get_plugin()->ExitFARW(Info);
}

/// Panel

#ifndef DEBUG

///=================================================================================================
namespace {

	typedef void (*FAtExit)(void);

	const int64_t MAX_ATEXITLIST_ENTRIES = 8;

	int64_t atexit_index = MAX_ATEXITLIST_ENTRIES - 1;
	FAtExit pf_atexitlist[MAX_ATEXITLIST_ENTRIES];

	void init_atexit()
	{
	}

	void invoke_atexit()
	{
		LogTrace();

		if (atexit_index < 0)
			atexit_index = 0;
		else
			++atexit_index;

		for (int64_t i = atexit_index; i < MAX_ATEXITLIST_ENTRIES; ++i)
		{
			LogDebug(L"[%I64d] ptr: %p\n", i, pf_atexitlist[i]);
			(*pf_atexitlist[i])();
		}
	}

}

extern "C" {

	BOOL WINAPI	DllMainCRTStartup(HANDLE, DWORD dwReason, PVOID) {
		switch (dwReason) {
			case DLL_PROCESS_ATTACH:
				init_atexit();
				break;

			case DLL_PROCESS_DETACH:
				invoke_atexit();
				break;
		}
		return true;
	}

	int atexit(FAtExit pf)
	{
		LogTrace();
		int64_t ind = ::InterlockedExchangeAdd64(&atexit_index, -1);
		if (ind >= 0)
		{
			LogDebug(L"[%I64d] ptr: %p\n", ind, pf);
			pf_atexitlist[ind] = pf;
			return 0;
		}
		return -1;
	}

	void __cxa_pure_virtual(void)
	{
//		::abort_message("pure virtual method called");
	}

}

#endif
