/**
	ontop: Always on top FAR3 plugin
	Switch between "always on top" state on/off

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

#include "farplugin.hpp"
#include "version.h"

#include <libbase/logger.hpp>

///========================================================================================== Export
void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	Base::Logger::set_target(Base::Logger::get_TargetToFile(L"c:/ontop.log"));
	Base::Logger::set_level(Base::Logger::LVL_TRACE);
	plugin.reset(new FarPlugin(psi));
}

void WINAPI GetPluginInfoW(PluginInfo * pi) {
	plugin->get_info(pi);
}

void WINAPI GetGlobalInfoW(GlobalInfo * info)
{
	using namespace AutoVersion;
	info->StructSize = sizeof(*info);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(MAJOR, MINOR, BUILD, REVISION, VS_RELEASE);
	info->Guid = FarPlugin::get_guid();
	info->Title = FarPlugin::get_name();
	info->Description = FarPlugin::get_description();
	info->Author = FarPlugin::get_author();
}

HANDLE WINAPI OpenW(const OpenInfo * Info) {
	return plugin->open(Info);
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
		::EnterCriticalSection(&cs);
		for (size_t i = atexit_index; i < MAX_ATEXITLIST_ENTRIES; ++i)
			(*pf_atexitlist[i])();
		::LeaveCriticalSection(&cs);
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
