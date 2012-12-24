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

#ifndef _LIBFAR_PLUGINSETTINGS_I_HPP_
#define _LIBFAR_PLUGINSETTINGS_I_HPP_

#include <libfar3/fwd.hpp>

namespace Far {

	///============================================================================ PluginSettings_i
	struct PluginSettings_i {
		virtual ~PluginSettings_i();

	public:
		// nvi
		bool load_settings()
		{
			return load_settings_();
		}

		bool save_settings() const
		{
			return save_settings_();
		}

	private:
		virtual bool load_settings_();

		virtual bool save_settings_() const;
	};

}

#endif
