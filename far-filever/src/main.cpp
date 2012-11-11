/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
	FAR3 plugin

	© 2012 Andrew Grechkin

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

#include <libbase/logger.hpp>


///========================================================================================== Export
/// GlobalInfo
void WINAPI GetGlobalInfoW(GlobalInfo * Info) {
	Base::Logger::set_target(Base::Logger::get_TargetToFile(L"D:/projects/FAR/FAR3/filever.log"));
	Base::Logger::set_level(Base::Logger::Level::Trace);

	LogTrace();
	FarGlobalInfo::inst().GetGlobalInfoW(Info);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo * Info) {
	LogTrace();
	return FarGlobalInfo::inst().ConfigureW(Info);
}

void WINAPI SetStartupInfoW(const PluginStartupInfo * Info) {
	LogTrace();
	FarGlobalInfo::inst().SetStartupInfoW(Info);
}


/// Plugin
void WINAPI GetPluginInfoW(PluginInfo * Info) {
	FarGlobalInfo::inst().get_plugin()->GetPluginInfoW(Info);
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	LogTrace();
	return FarGlobalInfo::inst().get_plugin()->OpenW(Info);
}

void WINAPI ExitFARW(const struct ExitInfo *Info) {
	LogTrace();
	FarGlobalInfo::inst().get_plugin()->ExitFARW(Info);
}


///=================================================================================================
namespace {

	typedef void (*FAtExit)(void);

	const size_t MAX_ATEXITLIST_ENTRIES = 1024;

	size_t atexit_index;
	FAtExit pf_atexitlist[MAX_ATEXITLIST_ENTRIES];
	CRITICAL_SECTION cs;


	void init_atexit()
	{
		atexit_index = MAX_ATEXITLIST_ENTRIES - 1;
		::InitializeCriticalSection(&cs);
	}

	void invoke_atexit()
	{
		for (size_t i = atexit_index; i < MAX_ATEXITLIST_ENTRIES; ++i)
			(*pf_atexitlist[i])();
		::DeleteCriticalSection(&cs);
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
		::EnterCriticalSection(&cs);

		int result = -1;
		if (atexit_index)
		{
			if (pf)
				pf_atexitlist[atexit_index--] = pf;
			result = 0;
		}

		::LeaveCriticalSection(&cs);

		return result;
	}

	void __cxa_pure_virtual(void)
	{
		//		::abort_message("pure virtual method called");
	}

}
