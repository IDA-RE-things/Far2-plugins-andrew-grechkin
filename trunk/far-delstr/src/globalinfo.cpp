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
#include <guid.hpp>
#include <lang.hpp>

#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libfar3/editor.hpp>
#include <libfar3/settings.hpp>

#include <libbase/pcstr.hpp>
#include <libbase/logger.hpp>

#include <version.h>

FarGlobalInfo::FarGlobalInfo() :
	rbValue_operation(0),
	cbValue_ismask(0)
{
	LogTrace();
	text[0] = 0;
	prefix[0] = 0;
	m_first_line = 0;
}

FarGlobalInfo::~FarGlobalInfo()
{
	LogTrace();
}

PCWSTR FarGlobalInfo::get_author() const
{
	return L"© 2013 Andrew Grechkin";
}

PCWSTR FarGlobalInfo::get_description() const
{
	return L"Delete strings in editor. FAR3lua plugin";
}

const GUID * FarGlobalInfo::get_guid() const
{
	return &PluginGuid;
}

PCWSTR FarGlobalInfo::get_title() const
{
	return L"delstr";
}

VersionInfo FarGlobalInfo::get_version() const
{
	using namespace AutoVersion;
	return MAKEFARVERSION(MAJOR, MINOR, BUILD, FARMANAGERVERSION_BUILD, VS_RELEASE);
}

VersionInfo FarGlobalInfo::get_min_version() const
{
	return MAKEFARVERSION(3, 0, 0, 3000, VS_RELEASE);
}

Far::Plugin_i * FarGlobalInfo::CreatePlugin(const PluginStartupInfo * Info) const
{
	LogTrace();
	Far::Plugin_i * plugin = create_FarPlugin(Info);
	return plugin;
}

void FarGlobalInfo::load_settings()
{
	LogTrace();
	Far::Settings_t settings(*get_guid());
	rbValue_operation = settings.get(L"operation", (int64_t)rbValue_operation);
	cbValue_ismask = settings.get(L"is_text_mask", (int64_t)cbValue_ismask);
	wcsncpy(prefix, settings.get(L"Prefix", L"delstr"), Base::lengthof(prefix));
	LogNoise(L"op: %Id\n", rbValue_operation);
	LogNoise(L"opm: %Id\n", cbValue_ismask);
}

void FarGlobalInfo::save_settings() const
{
	LogTrace();
	Far::Settings_t settings(*get_guid());
	settings.set(L"operation", (int64_t)rbValue_operation);
	settings.set(L"is_text_mask", (int64_t)cbValue_ismask);
	settings.set(L"Prefix", prefix);
	LogNoise(L"op: %Id\n", rbValue_operation);
	LogNoise(L"opm: %Id\n", cbValue_ismask);
}

void FarGlobalInfo::load_editor_info()
{
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
