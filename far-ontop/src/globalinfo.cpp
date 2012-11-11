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

#include <globalinfo.hpp>

#include <libfar3/helper.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <farplugin.hpp>
#include <guid.hpp>
#include <version.h>


FarGlobalInfo & FarGlobalInfo::inst() {
	static FarGlobalInfo ret;
	return ret;
}

FarGlobalInfo::FarGlobalInfo()
{
	LogTrace();
	Base::copy_str(Prefix, L"ontop");
}

FarGlobalInfo::~FarGlobalInfo() {
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2012 Andrew Grechkin";
}

PCWSTR FarGlobalInfo::get_description() const {
	return L"Always on top FAR3 plugin";
}

const GUID * FarGlobalInfo::get_guid() const {
	return &PluginGuid;
}

PCWSTR FarGlobalInfo::get_title() const {
	return L"ontop";
}

VersionInfo FarGlobalInfo::get_version() const {
	using namespace AutoVersion;
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, 0, VS_RC);
}

int FarGlobalInfo::Configure(const ConfigureInfo * /*Info*/) {
	return false;
}

Far::Plugin_i * FarGlobalInfo::CreatePlugin(const PluginStartupInfo * Info) {
	Far::Plugin_i * plugin = create_FarPlugin(this, Info);
	FarGlobalInfo::inst().load_settings();
	return plugin;
}

void FarGlobalInfo::load_settings() {
}

void FarGlobalInfo::save_settings() const {
}
