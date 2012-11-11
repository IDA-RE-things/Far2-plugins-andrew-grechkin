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

#include <libfar3/settings.hpp>
#include <libfar3/helper.hpp>

#include <libbase/logger.hpp>

namespace Far {

	///================================================================================== Settings_t
	Settings_t::~Settings_t()
	{
		psi().SettingsControl(m_hndl, SCTL_FREE, 0, 0);
	}

	Settings_t::Settings_t(const GUID & guid) :
		m_hndl(INVALID_HANDLE_VALUE )
	{
		FarSettingsCreate settings = {
		    sizeof(FarSettingsCreate),
		    guid,
		    m_hndl};
		if (psi().SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &settings))
			m_hndl = settings.Handle;
	}

	int Settings_t::create_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root)
	{
		FarSettingsValue value = {
		    sizeof(value),
		    root,
		    name};
		return (int)psi().SettingsControl(m_hndl, SCTL_CREATESUBKEY, 0, &value);
	}

	int Settings_t::open_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root) const
	{
		FarSettingsValue value = {
		    sizeof(value),
		    root,
		    name};
		return (int)psi().SettingsControl(m_hndl, SCTL_OPENSUBKEY, 0, &value);
	}

	bool Settings_t::del(FARSETTINGS_SUBFOLDERS root)
	{
		FarSettingsValue value = {
		    sizeof(value),
		    root,
		    nullptr};
		return psi().SettingsControl(m_hndl, SCTL_DELETE, 0, &value);
	}

	bool Settings_t::del(PCWSTR name, FARSETTINGS_SUBFOLDERS root)
	{
		FarSettingsValue value = {
		    sizeof(value),
		    root,
		    name};
		return psi().SettingsControl(m_hndl, SCTL_DELETE, 0, &value);
	}

	size_t Settings_t::get(PCWSTR name, PVOID value, size_t size, FARSETTINGS_SUBFOLDERS root) const
	{
		FarSettingsItem item = {
		    sizeof(item),
		    root,
		    name,
		    FST_DATA};
		if (psi().SettingsControl(m_hndl, SCTL_GET, 0, &item)) {
			if (value) {
				size = (item.Data.Size > size) ? size : item.Data.Size;
				memcpy(value, item.Data.Data, size);
				return size;
			} else {
				return item.Data.Size;
			}
		}
		return 0;
	}

	PCWSTR Settings_t::get(PCWSTR name, PCWSTR def, FARSETTINGS_SUBFOLDERS root) const
	{
		FarSettingsItem item = {
		    sizeof(item),
		    root,
		    name,
		    FST_STRING};
		return psi().SettingsControl(m_hndl, SCTL_GET, 0, &item) ? item.String : def;
	}

	uint64_t Settings_t::get(PCWSTR name, uint64_t def, FARSETTINGS_SUBFOLDERS root) const
	{
		FarSettingsItem item = {
		    sizeof(item),
		    root,
		    name,
		    FST_QWORD};
		return psi().SettingsControl(m_hndl, SCTL_GET, 0, &item) ? item.Number : def;
	}

	int64_t Settings_t::get(PCWSTR name, int64_t def, FARSETTINGS_SUBFOLDERS root) const
	{
		return (int64_t)get(name, (uint64_t)def, root);
	}

	uint32_t Settings_t::get(PCWSTR name, uint32_t def, FARSETTINGS_SUBFOLDERS root) const
	{
		return (uint32_t)get(name, (uint64_t)def, root);
	}

	int32_t Settings_t::get(PCWSTR name, int32_t def, FARSETTINGS_SUBFOLDERS root) const
	{
		return (int32_t)get(name, (uint64_t)def, root);
	}

	bool Settings_t::get(PCWSTR name, bool def, FARSETTINGS_SUBFOLDERS root) const
	{
		return get(name, def ? 1ull : 0ull, root);
	}

	bool Settings_t::set(PCWSTR name, PCVOID value, size_t size, FARSETTINGS_SUBFOLDERS root)
	{
		FarSettingsItem item = {
		    sizeof(item),
		    root,
		    name,
		    FST_DATA};
		item.Data.Size = size;
		item.Data.Data = value;
		return psi().SettingsControl(m_hndl, SCTL_SET, 0, &item);
	}

	bool Settings_t::set(PCWSTR name, PCWSTR value, FARSETTINGS_SUBFOLDERS root)
	{
		FarSettingsItem item = {
		    sizeof(item),
		    root,
		    name,
		    FST_STRING};
		item.String = value;
		return psi().SettingsControl(m_hndl, SCTL_SET, 0, &item);
	}

	bool Settings_t::set(PCWSTR name, uint64_t value, FARSETTINGS_SUBFOLDERS root)
	{
		FarSettingsItem item = {
		    sizeof(item),
		    root,
		    name,
		    FST_QWORD};
		item.Number = value;
		return psi().SettingsControl(m_hndl, SCTL_SET, 0, &item);
	}

}
