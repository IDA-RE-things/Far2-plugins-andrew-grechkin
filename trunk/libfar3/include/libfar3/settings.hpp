#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include <libfar3/plugin.hpp>
#include <libbase/std.hpp>


namespace Far {

	///================================================================================== Settings_t
	struct Settings_t {
		~Settings_t();

		Settings_t(const GUID & guid);

		int create_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		int open_key(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool del(FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool del(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		size_t get(PCWSTR name, PVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		PCWSTR get(PCWSTR name, PCWSTR def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		uint64_t get(PCWSTR name, uint64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		int64_t get(PCWSTR name, int64_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		uint32_t get(PCWSTR name, uint32_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		int32_t get(PCWSTR name, int32_t def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool get(PCWSTR name, bool def, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT) const;

		bool set(PCWSTR name, PCVOID value, size_t size, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, PCWSTR value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

		bool set(PCWSTR name, uint64_t value, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);

	private:
		HANDLE m_hndl;
	};

}


#endif
