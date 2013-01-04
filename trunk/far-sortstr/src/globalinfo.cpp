/**
	sortstr: Sort strings in editor
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
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libfar3/editor.hpp>
#include <libfar3/settings.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <version.h>

FarGlobalInfo::FarGlobalInfo():
	cbValue_Operation(0)
{
	LogTrace();
	cbValue_Invert = cbValue_Sensitive = cbValue_Numeric = cbValue_Selected = cbValue_AsEmpty = 0;
	edValue_Whitespaces[0] = 0;
	prefix[0] = 0;
	m_first_line = 0;
}

FarGlobalInfo::~FarGlobalInfo() {
	LogTrace();
}

PCWSTR FarGlobalInfo::get_author() const {
	return L"© 2012 Andrew Grechkin";
}

PCWSTR FarGlobalInfo::get_description() const {
	return L"Sort strings in editor. FAR3 plugin";
}

const GUID * FarGlobalInfo::get_guid() const {
	return &PluginGuid;
}

PCWSTR FarGlobalInfo::get_title() const {
	return L"sortstr";
}

VersionInfo FarGlobalInfo::get_min_version() const {
	using namespace AutoVersion;
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, 0, VS_RC);
}

Far::Plugin_i * FarGlobalInfo::CreatePlugin(const PluginStartupInfo * Info) const {
	LogTrace();
	Far::Plugin_i * plugin = create_FarPlugin(Info);
	return plugin;
}

void FarGlobalInfo::load_settings() {
	LogTrace();
	Far::Settings_t settings(*get_guid());
	cbValue_Invert = settings.get(L"invert", (int64_t)cbValue_Invert);
	cbValue_Sensitive = settings.get(L"case", (int64_t)cbValue_Sensitive);
	cbValue_Numeric = settings.get(L"numeric", (int64_t)cbValue_Numeric);
	cbValue_Selected = settings.get(L"selection", (int64_t)cbValue_Selected);
	cbValue_AsEmpty = settings.get(L"asempty", (int64_t)cbValue_AsEmpty);
	wcsncpy(edValue_Whitespaces, settings.get(L"whitespaces", L" "), Base::lengthof(edValue_Whitespaces));
	wcsncpy(prefix, settings.get(L"Prefix", L"sortstr"), Base::lengthof(prefix));

	LogDebug(L"cbValue_Invert: %Id\n", cbValue_Invert);
	LogDebug(L"cbValue_Sensitive: %Id\n", cbValue_Sensitive);
	LogDebug(L"cbValue_Numeric: %Id\n", cbValue_Numeric);
	LogDebug(L"cbValue_Selected: %Id\n", cbValue_Selected);
	LogDebug(L"cbValue_AsEmpty: %Id\n", cbValue_AsEmpty);
	LogDebug(L"edValue_Whitespaces: '%s'\n", edValue_Whitespaces);
	LogDebug(L"prefix: '%s'\n", prefix);

	cbValue_Operation = 0;
}

void FarGlobalInfo::save_settings() const {
	LogTrace();
	Far::Settings_t settings(*get_guid());
	settings.set(L"invert", (int64_t)cbValue_Invert);
	settings.set(L"case", (int64_t)cbValue_Sensitive);
	settings.set(L"numeric", (int64_t)cbValue_Numeric);
	settings.set(L"selection", (int64_t)cbValue_Selected);
	settings.set(L"asempty", (int64_t)cbValue_AsEmpty);
	settings.set(L"whitespace", edValue_Whitespaces);
	settings.set(L"Prefix", prefix);
	LogDebug(L"cbValue_Invert: %Id\n", cbValue_Invert);
	LogDebug(L"cbValue_Sensitive: %Id\n", cbValue_Sensitive);
	LogDebug(L"cbValue_Numeric: %Id\n", cbValue_Numeric);
	LogDebug(L"cbValue_Selected: %Id\n", cbValue_Selected);
	LogDebug(L"cbValue_AsEmpty: %Id\n", cbValue_AsEmpty);
	LogDebug(L"edValue_Whitespaces: '%s'\n", edValue_Whitespaces);
	LogDebug(L"prefix: '%s'\n", prefix);
}

void FarGlobalInfo::load_editor_info() {
	LogTrace();
	Far::Editor::get_info(m_ei);

	if (m_ei.BlockType == BTYPE_STREAM || m_ei.BlockType == BTYPE_COLUMN)
		m_first_line = m_ei.BlockStartLine;
	else
		m_first_line = 0;
}

FarGlobalInfo * get_global_info()
{
	return (FarGlobalInfo*)Far::helper_t::inst().get_global_info();
}
