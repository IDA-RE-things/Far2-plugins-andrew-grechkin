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

#ifndef _LIBFAR_PLUGIN_I_HPP_
#define _LIBFAR_PLUGIN_I_HPP_

#include <libfar3/plugin.hpp>
#include <libfar3/fwd.hpp>

namespace Far {

	///==================================================================================== Plugin_i
	struct Plugin_i {
	public:
		Plugin_i(const PluginStartupInfo * Info);

		virtual ~Plugin_i();

	public: // nvi
		void GetPluginInfoW(PluginInfo * Info);

		PanelController_i * OpenW(const OpenInfo * Info);

		void ExitFARW(const ExitInfo * Info);

		const PluginStartupInfo & psi() const {return m_psi;}

		const FarStandardFunctions & fsf() const {return m_fsf;}

	private:
		virtual void GetPluginInfo(PluginInfo * pi) = 0;

		virtual PanelController_i * Open(const OpenInfo * Info);

		virtual void ExitFAR(const ExitInfo * Info);

	private:
		PluginStartupInfo m_psi;
		FarStandardFunctions m_fsf;
	};

}

#endif
