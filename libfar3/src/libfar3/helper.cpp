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

#include <libfar3/helper.hpp>

#include <libbase/logger.hpp>
#include <libbase/memory.hpp>

namespace Far {

	///==================================================================================== helper_t
	helper_t & helper_t::inst() {
		static helper_t ret;
		return ret;
	}

	helper_t & helper_t::init(const GUID & guid, const PluginStartupInfo * psi) {
		m_guid = guid;
		m_psi = *psi;
		m_fsf = *psi->FSF;
		m_psi.FSF = &m_fsf;
		return *this;
	}

	const GUID * helper_t::guid() const {
		return &m_guid;
	}

	const PluginStartupInfo & helper_t::psi() const {
		return m_psi;
	}

	const FarStandardFunctions & helper_t::fsf() const {
		return m_fsf;
	}

	helper_t::helper_t() {
	}


	///=============================================================================================
	PCWSTR get_msg(ssize_t MsgId) {
		return (MsgId == -1) ? Base::EMPTY_STR : psi().GetMsg(get_plugin_guid(), MsgId);
	}

}
