#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_def/std.h>
#include <libwin_def/shared_ptr.h>

#ifndef FAR2
#	include <libwin_def/str.h>
#	include <API_far3/helper.hpp>
#else
#	include <libwin_def/reg.h>
#	include <API_far2/helper.hpp>
#endif

// main dialog parameters
enum {
	HEIGHT = 11,
	WIDTH = 70,

	indDelAll = 1,
	indDelRep,
	indDelWithText,
	indDelWithoutText,
	indText,
	indIsMask,
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

	PCWSTR get_prefix() const;

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();
};

extern winstd::shared_ptr<FarPlugin> plugin;

#endif
