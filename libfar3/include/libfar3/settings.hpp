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

#ifndef _LIBFAR_SETTINGS_HPP_
#define _LIBFAR_SETTINGS_HPP_

#include <libfar3/plugin.hpp>
#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>

namespace Far {

	struct Settings_t: private Base::Uncopyable {
		~Settings_t();

		Settings_t(const GUID & guid);

		intptr_t create_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		intptr_t open_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		intptr_t del(PCWSTR name = nullptr, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		size_t get(PCWSTR name, PVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		PCWSTR get(PCWSTR name, PCWSTR def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		uint64_t get(PCWSTR name, uint64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		int64_t get(PCWSTR name, int64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool get(PCWSTR name, bool def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool set(PCWSTR name, PCVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, PCWSTR value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, uint64_t value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, int64_t value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, bool value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

	private:
		HANDLE m_hndl;
	};

}

#endif
