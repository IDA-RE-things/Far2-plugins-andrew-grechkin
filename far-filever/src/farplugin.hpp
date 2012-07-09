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

#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libbase/std.hpp>
#include <libbase/logger.hpp>
#include <libbase/shared_ptr.hpp>
#include <libbase/str.hpp>

#include <libfar3/helper.hpp>


struct FarPlugin;
extern Base::shared_ptr<FarPlugin> plugin;

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

	HANDLE open(const OpenInfo * Info);

	PCWSTR get_prefix() const;

	static GUID get_guid();

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();
};

#endif
