﻿/**
	rcl2apps: Maps mouse button to Apps key
	FAR2, FAR3 plugin

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

///========================================================================================== Export
#ifndef FAR2
void WINAPI ExitFARW(const ExitInfo * /*Info*/)
#else
void WINAPI ExitFARW()
#endif
{
	RtlHookImportTable("kernel32.dll", "ReadConsoleInputW", (PROC)Real_ReadConsoleInputW, ::GetModuleHandleW(nullptr));
}

void WINAPI SetStartupInfoW(const PluginStartupInfo * psi) {
	plugin.reset(new FarPlugin(psi));
}

void WINAPI GetPluginInfoW(PluginInfo * pi) {
	plugin->get_info(pi);
}

#ifndef FAR2
void WINAPI GetGlobalInfoW(GlobalInfo * info)
{
	using namespace AutoVersion;
	info->StructSize = sizeof(*info);
	info->MinFarVersion = FARMANAGERVERSION;
	info->Version = MAKEFARVERSION(MAJOR,MINOR,BUILD,REVISION,VS_RELEASE);
	info->Guid = FarPlugin::get_guid();
	info->Title = FarPlugin::get_name();
	info->Description = FarPlugin::get_description();
	info->Author = FarPlugin::get_author();
}
#else
int WINAPI GetMinFarVersionW() {
	return	MAKEFARVERSION(MIN_FAR_VERMAJOR, MIN_FAR_VERMINOR, MIN_FAR_BUILD);
}
#endif

extern "C" {
	BOOL WINAPI DllMainCRTStartup(HANDLE /*hDll*/, DWORD dwReason, LPVOID /*lpReserved*/) {
		static HANDLE hInsuranceEvent = INVALID_HANDLE_VALUE;
		if (dwReason == DLL_PROCESS_ATTACH) {
			WCHAR lpEventName[MAX_PATH];
			WCHAR lpProcessId[MAX_PATH];
			as_cstr(lpProcessId, GetCurrentProcessId(), 16);
			Copy(lpEventName, L"__RCL2APPS__", lengthof(lpEventName));
			Cat(lpEventName, lpProcessId, lengthof(lpEventName));
			hInsuranceEvent = ::CreateEvent(nullptr, false, false, lpEventName);
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				::SetEvent(hInsuranceEvent);
				return false;
			}
		}

		if (dwReason == DLL_PROCESS_DETACH)
			::CloseHandle(hInsuranceEvent);

		return true;
	}
}
