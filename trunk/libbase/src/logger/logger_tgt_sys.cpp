#include <libbase/lock.hpp>
#include <libbase/logger.hpp>
#include <libbase/memory.hpp>
#include <libbase/pcstr.hpp>

#include "event_msg.h"

namespace Base {
	namespace Logger {

		WORD const LogLevelTypes[(int)Level::Fatal + 1] = {
			EVENTLOG_SUCCESS,
			EVENTLOG_SUCCESS,
			EVENTLOG_INFORMATION_TYPE,
			EVENTLOG_INFORMATION_TYPE,
			EVENTLOG_WARNING_TYPE,
			EVENTLOG_WARNING_TYPE,
			EVENTLOG_ERROR_TYPE,
			EVENTLOG_ERROR_TYPE,
		};

		struct LogToSys: public Target_i {
			virtual ~LogToSys();

			virtual void out(const Module_i * lgr, Level lvl, PCWSTR str, size_t size) const;

			virtual void out(PCWSTR str, size_t size) const;

			LogToSys(PCWSTR name, PCWSTR path);

			static void app_register(PCWSTR name, PCWSTR path);

		private:
			HANDLE	m_hndl;
		};


		LogToSys::~LogToSys() {
			::DeregisterEventSource(m_hndl);
		}

		LogToSys::LogToSys(PCWSTR name, PCWSTR path) {
			app_register(name, path);
			m_hndl = ::RegisterEventSourceW(nullptr, name);
		}

		void LogToSys::out(const Module_i * /*lgr*/, Level lvl, PCWSTR str, size_t /*size*/) const {
//			PSID user = nullptr;
//			HANDLE token;
//			PTOKEN_USER token_user = nullptr;
//			if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &token)) {
//				token_user = (PTOKEN_USER)DefaultTokenInformation(token, TokenUser);
//				if (token_user)
//					user = token_user->User.Sid;
//				CloseHandle(token);
//			}
			::ReportEventW(m_hndl, LogLevelTypes[(int)lvl], 0, EV_MSG_STRING, nullptr, 1, 0, &str, nullptr);
//			free(token_user);
		}

		void LogToSys::out(PCWSTR str, size_t /*size*/) const {
			::ReportEventW(m_hndl, LogLevelTypes[(int)get_default_level()], 0, EV_MSG_STRING, nullptr, 1, 0, &str, nullptr);
		}

		void LogToSys::app_register(PCWSTR name, PCWSTR path) {
			WCHAR path_buf[MAX_PATH_LEN], * fullpath = path_buf;
			if (is_str_empty(path)) {
				::GetModuleFileNameW(0, path_buf, lengthof(path_buf));
			} else {
				fullpath = (PWSTR)path;
			}

			WCHAR key[MAX_PATH_LEN];
			copy_str(key, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\", lengthof(key));
			cat_str(key, name, lengthof(key));
			HKEY hKey = nullptr;
			::RegCreateKeyW(HKEY_LOCAL_MACHINE, key, &hKey);
			// Add the Event ID message-file name to the subkey.
			::RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)fullpath, (DWORD)((get_str_len(fullpath) + 1) * sizeof(WCHAR)));
			// Set the supported types flags.
			DWORD dwData = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
			::RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(dwData));
			::RegCloseKey(hKey);
		}


		Target_i * get_TargetToSys(PCWSTR name, PCWSTR path) {
			return new LogToSys(name, path);
		}

	}
}
