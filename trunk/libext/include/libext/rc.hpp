#ifndef _LIBEXT_RC_HPP_
#define _LIBEXT_RC_HPP_

#include <libbase/std.hpp>

namespace Ext {

	///============================================================================ RemoteConnection
	struct RemoteConnection {
		~RemoteConnection();

		RemoteConnection(const ustring & host = ustring(), PCWSTR user = nullptr, PCWSTR pass = nullptr);

		void connect(const ustring & host = ustring(), PCWSTR user = nullptr, PCWSTR pass = nullptr);

		void disconnect();

		ustring get_host() const;

	private:
		ustring m_host;
		bool m_connected;
	};

}

#endif
