#include <libfar3/helper.hpp>


namespace Far {

	void ibox(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text};
		psi().Message(get_plugin_guid(), nullptr, FMSG_MB_OK, nullptr, Msg, Base::lengthof(Msg), 1);
	}

	void mbox(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text};
		psi().Message(get_plugin_guid(), nullptr, FMSG_MB_OK, nullptr, Msg, Base::lengthof(Msg), 1);
	}

	void ebox(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text};
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_MB_OK, nullptr, Msg, Base::lengthof(Msg), 1);
	}

	void ebox(PCWSTR msgs[], size_t size, PCWSTR help) {
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_MB_OK, help, msgs, size, 1);
	}

	void ebox(const Base::mstring & msg, PCWSTR title) {
		PCWSTR tmp[msg.size() + 1];
		tmp[0] = title;
		for (size_t i = 0; i < msg.size(); ++i) {
			tmp[i + 1] = msg[i];
		}
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_MB_OK, nullptr, tmp, sizeofa(tmp), 1);
	}

	void ebox(DWORD err) {
//		ustring title(L"Error: ");
//		title += Base::as_str(err);
		::SetLastError(err);
//		PCWSTR Msg[] = {title.c_str(), L"OK", };
		psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_ERRORTYPE | FMSG_MB_OK, nullptr, nullptr, 0, 0);
	}

	bool question(PCWSTR text, PCWSTR tit) {
		PCWSTR Msg[] = {tit, text};
		return psi().Message(get_plugin_guid(), nullptr, FMSG_WARNING | FMSG_MB_OKCANCEL, nullptr, Msg, Base::lengthof(Msg), 2) == 0;
	}

}
