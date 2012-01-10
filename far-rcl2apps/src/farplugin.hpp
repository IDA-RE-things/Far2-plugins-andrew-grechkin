#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_def/std.h>
#include <libwin_def/shared_ptr.h>

#ifndef FAR2
#	include <API_far3/helper.hpp>
#else
#	include <API_far2/helper.hpp>
#endif

typedef BOOL (WINAPI *FReadConsoleInputW)(HANDLE, PINPUT_RECORD, DWORD, PDWORD);

extern FReadConsoleInputW Real_ReadConsoleInputW;

PROC RtlHookImportTable(PCSTR lpModuleName, PCSTR lpFunctionName, PROC pfnNew, HMODULE hModule);

BOOL WINAPI Thunk_ReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, PDWORD lpNumberOfEventsRead);

struct Options {
	Options();

	DWORD ControlKeyState;
	DWORD Button;
//	LONG Top, Bottom;
	BOOL Flag;

#ifndef FAR2
	void load();
#else
	void load(const ustring & path);
#endif
};

struct FarPlugin {
	FarPlugin(const PluginStartupInfo * psi);

	void get_info(PluginInfo * pi) const;

#ifndef FAR2
	HANDLE open(const OpenInfo * Info);

	static GUID get_guid();
#else
	HANDLE open(int OpenFrom, INT_PTR Item);
#endif

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();

	PCWSTR get_prefix() const;

	void process_input(PINPUT_RECORD InRec);

	bool is_panel(COORD Pos) const;

private:
	Options options;
};

extern winstd::shared_ptr<FarPlugin> plugin;

#endif
