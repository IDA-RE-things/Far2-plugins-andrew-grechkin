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

#ifndef _LIBFAR_HELPER_HPP_
#define _LIBFAR_HELPER_HPP_

#include <libfar3/plugin.hpp>
#include <libfar3/globalinfo_i.hpp>
#include <libfar3/plugin_i.hpp>

#include <libbase/std.hpp>
#include <libbase/mstring.hpp>
#include <libbase/memory.hpp>

namespace Far {

	enum {
		MenuTitle,
		DiskTitle,
		DlgTitle,

		txtBtnOk,
		txtBtnCancel,
	};

	///==================================================================================== helper_t
	struct helper_t {
		static helper_t & inst()
		{
			static helper_t ret;
			return ret;
		}

		~helper_t()
		{
			delete m_gi;
		}

		helper_t & init(GlobalInfo_i * gi)
		{
			m_gi = gi;
			return *this;
		}

		const GUID * get_guid() const
		{
			return m_gi->get_guid();
		}

		const PluginStartupInfo & psi() const
		{
			return get_plugin()->psi();
		}

		const FarStandardFunctions & fsf() const
		{
			return get_plugin()->fsf();
		}

		GlobalInfo_i * get_global_info() const
		{
			return m_gi;
		}

		Plugin_i * get_plugin() const
		{
			return m_gi->get_plugin();
		}

	private:
		helper_t() :
			m_gi(nullptr)
		{
		}

		GlobalInfo_i * m_gi;
	};

	inline const GUID * get_plugin_guid()
	{
		return helper_t::inst().get_guid();
	}

	inline const PluginStartupInfo & psi()
	{
		return helper_t::inst().psi();
	}

	inline const FarStandardFunctions & fsf()
	{
		return helper_t::inst().fsf();
	}

	///=============================================================================================
	PCWSTR get_msg(ssize_t MsgId);

	void ibox(PCWSTR text, PCWSTR tit = L"Info");

	void mbox(PCWSTR text, PCWSTR tit = L"Message");

	void ebox(PCWSTR text, PCWSTR tit = L"Error");

	void ebox(PCWSTR msgs[], size_t size, PCWSTR help = nullptr);

	void ebox(const Base::mstring & msg, PCWSTR title = Base::EMPTY_STR);

	void ebox(DWORD err);

	bool question(PCWSTR text, PCWSTR tit);

//	///================================================================================= KeyAction_t
//	typedef bool (PanelController_i::*PanelMemFun)();
//
//	struct KeyAction_t {
//		FarKey Key;
//		PCWSTR Text;
//		PCWSTR LongText;
//		PanelMemFun Handler;
//	};

///============================================================================== ProgressWindow
	struct ProgressWindow {
		ProgressWindow(size_t /*size*/, PCWSTR /*title*/)
		{
		}

		void set_name(size_t /*num*/, PCWSTR /*name*/)
		{
		}
	private:
	};

//	inline uint64_t get_panel_settings() {
//		return psi().AdvControl(get_plugin_guid(), ACTL_GETPANELSETTINGS, 0, nullptr);
//	} GetSetting(FSSF_PANEL,L"ShowHidden")?true:false;

//	inline uint64_t get_interface_settings() {
//		return psi().AdvControl(get_plugin_guid(), ACTL_GETINTERFACESETTINGS, 0, nullptr);
//	}

}

#endif
