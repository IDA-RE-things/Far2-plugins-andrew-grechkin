#ifndef _LIBCOM_GUID_HPP_
#define _LIBCOM_GUID_HPP_

#include <libbase/std.hpp>


namespace Com {

	struct WinGUID: public GUID {
		WinGUID();

		WinGUID(PCWSTR str) {
			init(str);
		}
		WinGUID(const ustring & str) {
			init(str);
		}
		WinGUID(const PROPVARIANT & prop) {
			init(prop);
		}

		void init(PCWSTR str);
		void init(const ustring & str);
		void init(const PROPVARIANT & prop);

		ustring as_str() const {
			return WinGUID::as_str(*this);
		}

		static ustring as_str(const GUID & guid);
	};


	inline ustring as_str(const GUID &guid) {
		return WinGUID::as_str(guid);
	}

}


#endif
