/**
 © 2012 Andrew Grechkin
 Source code: <http://code.google.com/p/andrew-grechkin>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include <libfar3/globalinfo_i.hpp>
#include <libfar3/plugin_i.hpp>

#include <libbase/std.hpp>
#include <libbase/logger.hpp>

namespace Far {

	///=============================================================================================
	void GlobalInfo_i::GetGlobalInfoW(GlobalInfo * Info) const
	{
		LogTrace();
		Info->StructSize = sizeof(*Info);
		Info->MinFarVersion = get_min_version();
		Info->Version = get_version();
		Info->Author = get_author();
		Info->Description = get_description();
		Info->Guid = *get_guid();
		Info->Title = get_title();
	}

	intptr_t GlobalInfo_i::ConfigureW(const ConfigureInfo * Info)
	{
		LogTrace();
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return Configure(Info);
	}

	void GlobalInfo_i::SetStartupInfoW(const PluginStartupInfo * Info)
	{
		LogTrace();
		if (Info->StructSize < sizeof(*Info))
			return;
		if (!m_plugin)
			m_plugin = CreatePlugin(Info);
	}

	Plugin_i * GlobalInfo_i::get_plugin() const
	{
		return m_plugin;
	}

	///=============================================================================================
	GlobalInfo_i::GlobalInfo_i() :
		m_plugin(nullptr)
	{
		LogTrace();
	}

	GlobalInfo_i::~GlobalInfo_i()
	{
		delete m_plugin;
		LogTrace();
	}

	VersionInfo GlobalInfo_i::get_min_version() const
	{
		return FARMANAGERVERSION;
	}

	intptr_t GlobalInfo_i::Configure(const ConfigureInfo * /*Info*/)
	{
		LogTrace();
		return 0;
	}

}
