/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
	FAR3 plugin

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
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <version.h>

FarGlobalInfo::FarGlobalInfo()
{
	LogTrace();
	Base::Str::copy(prefix, L"fver");
}

FarGlobalInfo::~FarGlobalInfo() {
	LogTrace();
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2013 Andrew Grechkin";
}

PCWSTR FarGlobalInfo::get_description() const {
	return L"Displays version information from file resource";
}

const GUID * FarGlobalInfo::get_guid() const {
	return &PluginGuid;
}

PCWSTR FarGlobalInfo::get_title() const {
	return L"filever";
}

VersionInfo FarGlobalInfo::get_version() const {
	using namespace AutoVersion;
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, FARMANAGERVERSION_BUILD, VS_RELEASE);
}

VersionInfo FarGlobalInfo::get_min_version() const {
	return MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
}

Far::Plugin_i * FarGlobalInfo::CreatePlugin(const PluginStartupInfo * Info) const {
	LogTrace();
	Far::Plugin_i * plugin = create_FarPlugin(Info);
	return plugin;
}

void FarGlobalInfo::load_settings() {
	LogTrace();
}

void FarGlobalInfo::save_settings() const {
	LogTrace();
}

FarGlobalInfo * get_global_info()
{
	return (FarGlobalInfo*)Far::helper_t::inst().get_global_info();
}
