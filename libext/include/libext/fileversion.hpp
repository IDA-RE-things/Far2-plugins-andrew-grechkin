#ifndef _LIBEXT_FILEVERSION_HPP_
#define _LIBEXT_FILEVERSION_HPP_

#include <libbase/std.hpp>

namespace Ext {

	///================================================================================= FileVersion
	struct FileVersion {
		FileVersion(PCWSTR path);

		ustring get_version() const {
			return m_ver;
		}

	private:
		ustring m_ver;
	};

}

#endif
