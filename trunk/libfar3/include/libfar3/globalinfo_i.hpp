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

#ifndef _LIBFAR_GLOBALINFO_I_HPP_
#define _LIBFAR_GLOBALINFO_I_HPP_

#include <libfar3/plugin.hpp>
#include <libfar3/fwd.hpp>

namespace Far {

	///================================================================================ GlobalInfo_i
	struct GlobalInfo_i {
		GlobalInfo_i();

		virtual ~GlobalInfo_i();

	public:
		void GetGlobalInfoW(GlobalInfo * Info) const;

		intptr_t ConfigureW(const ConfigureInfo * Info);

		void SetStartupInfoW(const PluginStartupInfo * Info);

		Plugin_i * get_plugin() const;

	public:
		virtual PCWSTR get_author() const = 0;

		virtual PCWSTR get_description() const = 0;

		virtual const GUID * get_guid() const = 0;

		virtual PCWSTR get_title() const = 0;

		virtual VersionInfo get_min_version() const = 0;

		virtual intptr_t Configure(const ConfigureInfo * Info);

		virtual Plugin_i * CreatePlugin(const PluginStartupInfo * Info) const = 0;

	private:
		Plugin_i * m_plugin;
	};

}

#endif
