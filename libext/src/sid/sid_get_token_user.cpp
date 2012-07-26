#include <libext/sid.hpp>
#include <libext/exception.hpp>
#include <libbase/memory.hpp>

namespace Ext {

	ustring get_token_user(HANDLE hToken) {
		DWORD size = 0;
		if (!::GetTokenInformation(hToken, TokenUser, nullptr, 0, &size) && size) {
			Base::auto_buf<PTOKEN_USER> buf(size);
			CheckApi(::GetTokenInformation(hToken, TokenUser, buf, buf.size(), &size));
			return Sid::get_name(buf->User.Sid);
		}
		return ustring();
	}

}
