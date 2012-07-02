#ifndef _LIBEXT_RC_HPP_
#define _LIBEXT_RC_HPP_

#include <libbase/std.hpp>
#include <libbase/str.hpp>

namespace Ext {

	///============================================================================ RemoteConnection
	struct RemoteConnection {
		~RemoteConnection();

		RemoteConnection(PCWSTR host = nullptr, PCWSTR user = nullptr, PCWSTR pass = nullptr);

		void connect(PCWSTR host, PCWSTR user = nullptr, PCWSTR pass = nullptr);

		void disconnect();

		PCWSTR get_host() const;

	private:
		ustring m_host;
		bool m_connected;
	};

}

#endif
