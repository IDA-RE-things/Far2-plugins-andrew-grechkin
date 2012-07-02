#include <libbase/lock.hpp>
#include <libbase/logger.hpp>
#include <libbase/memory.hpp>
#include <libbase/pcstr.hpp>

namespace Base {
	namespace Logger {

		//	void		Write(DWORD Event, WORD Count, LPCWSTR *Strings) {
		//		PSID user = nullptr;
		//		HANDLE token;
		//		PTOKEN_USER token_user = nullptr;
		//		if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &token)) {
		//			token_user = (PTOKEN_USER)DefaultTokenInformation(token, TokenUser);
		//			if (token_user)
		//				user = token_user->User.Sid;
		//			CloseHandle(token);
		//		}
		//		ReportEventW(m_hndl, EVENTLOG_ERROR_TYPE, 0, Event, user, Count, 0, Strings, nullptr);
		//		free(token_user);
		//	}

//		void WinLog::Register(PCWSTR name, PCWSTR path) {
//			WCHAR	fullpath[MAX_PATH_LEN];
//			WCHAR	key[MAX_PATH_LEN];
//			if (!path || is_str_empty(path)) {
//				::GetModuleFileNameW(0, fullpath, sizeofa(fullpath));
//			} else {
//				copy_str(fullpath, path, sizeofa(fullpath));
//			}
//			HKEY	hKey = nullptr;
//			copy_str(key, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\", sizeofa(key));
//			cat_str(key, name, sizeofa(key));
//			::RegCreateKeyW(HKEY_LOCAL_MACHINE, key, &hKey);
//			// Add the Event ID message-file name to the subkey.
//			::RegSetValueExW(hKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)fullpath, (DWORD)((get_str_len(fullpath) + 1)*sizeof(WCHAR)));
//			// Set the supported types flags.
//			DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
//			::RegSetValueExW(hKey, L"TypesSupported", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(dwData));
//			::RegCloseKey(hKey);
//		}

		struct LogToSys: public Target_i {
			virtual ~LogToSys();

			virtual void out(const Module_i * lgr, Level lvl, PCWSTR str, size_t size) const;

			LogToSys(PCWSTR name);

			static void app_register(PCWSTR name, PCWSTR path = nullptr);

		private:
			HANDLE	m_hndl;
		};


		LogToSys::~LogToSys() {
			::DeregisterEventSource(m_hndl);
		}

		LogToSys::LogToSys(PCWSTR name):
			m_hndl(::RegisterEventSourceW(nullptr, name)) {
		}

		void LogToSys::out(const Module_i * /*lgr*/, Level /*lvl*/, PCWSTR str, size_t size) const {
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


		Target_i * get_TargetToSys(PCWSTR name) {
			return new LogToSys(name);
		}

	}
}
