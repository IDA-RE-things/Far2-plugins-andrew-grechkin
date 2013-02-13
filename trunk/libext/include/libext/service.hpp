/**
	win_svc

	@classes	()
	@author		© 2012 Andrew Grechkin
	@link		()
 **/

#ifndef WIN_SVC_HPP
#define WIN_SVC_HPP


#include <libext/rcfwd.hpp>

#include <libbase/std.hpp>
#include <libbase/memory.hpp>
#include <libbase/mstring.hpp>
//#include <libbase/command_p.hpp>


namespace Ext {

	struct Service: Base::Uncopyable {

		struct Manager;

		enum class EnumerateType_t: ssize_t {
			SERVICES = SERVICE_WIN32,
			DRIVERS = SERVICE_DRIVER,
//			ADAPTERS = SERVICE_ADAPTER,
		};

		enum class Type_t: ssize_t {
			KERNEL_DRIVER = SERVICE_KERNEL_DRIVER,
			FILE_SYSTEM_DRIVER = SERVICE_FILE_SYSTEM_DRIVER,
			ADAPTER = SERVICE_ADAPTER,
			RECOGNIZER_DRIVER = SERVICE_RECOGNIZER_DRIVER,
			WIN32_OWN_PROCESS = SERVICE_WIN32_OWN_PROCESS,
			WIN32_SHARE_PROCESS = SERVICE_WIN32_SHARE_PROCESS,
			INTERACTIVE_PROCESS = SERVICE_INTERACTIVE_PROCESS,
		};

		enum class State_t: ssize_t {
			STOPPED = 0x00000001,
			STARTING = 0x00000002,
			STOPPING = 0x00000003,
			STARTED = 0x00000004,
			CONTINUING = 0x00000005,
			PAUSING = 0x00000006,
			PAUSED = 0x00000007,
		};

		enum class Start_t: ssize_t {
			BOOT = SERVICE_BOOT_START,
			SYSTEM = SERVICE_SYSTEM_START,
			AUTO = SERVICE_AUTO_START,
			AUTO_DELAYED = SERVICE_AUTO_START | 0x10000,
			DEMAND = SERVICE_DEMAND_START,
			DISABLED = SERVICE_DISABLED,
		};

		enum class Error_t: ssize_t {
			IGNORE_ERROR = SERVICE_ERROR_IGNORE,
			NORMAL = SERVICE_ERROR_NORMAL,
			SEVERE = SERVICE_ERROR_SEVERE,
			CRITICAL = SERVICE_ERROR_CRITICAL,
		};

		struct Create_t {
			Create_t(const ustring & _name, const ustring & _binaryPathName);

			void set_type(Type_t n);

			void set_start(Start_t n);

			void set_error_control(Error_t n);

			void set_group(PCWSTR n);

			void set_tag(DWORD & n);

			void set_dependencies(PCWSTR n);

			void set_display_name(PCWSTR n);

			PCWSTR get_name() const;

		private:
			ustring name;
			DWORD serviceType;
			DWORD startType;
			DWORD errorControl;
			ustring binaryPathName;
			PCWSTR loadOrderGroup;
			PDWORD tagId;
			PCWSTR dependencies;
			PCWSTR displayName;

			friend struct Service::Manager;
		};


		struct Config_t {
			Config_t();

			void set_type(Type_t n, Type_t o);

			void set_start(Start_t n, Start_t o);

			void set_error_control(Error_t n, Error_t o);

			void set_path(PCWSTR n, PCWSTR o);

			void set_group(PCWSTR n, PCWSTR o);

			void set_tag(DWORD & n, DWORD o);

			void set_dependencies(PCWSTR n, PCWSTR o);

			void set_display_name(PCWSTR n, PCWSTR o);

		private:
			DWORD serviceType;
			DWORD startType;
			DWORD errorControl;
			PCWSTR binaryPathName;
			PCWSTR loadOrderGroup;
			PDWORD tagId;
			PCWSTR dependencies;
			PCWSTR displayName;

			friend struct Service;
		};


		struct Logon_t: public Config_t {
			Logon_t();

			Logon_t(PCWSTR user, PCWSTR pass = nullptr);

		private:
			PCWSTR serviceStartName;
			PCWSTR password;

			friend struct Service;
		};


		struct Status_t: public SERVICE_STATUS_PROCESS {
		};


		///================================================================================== Info_t
		struct Info_t {
			ustring name;
			ustring displayName;
			Start_t startType;
			Error_t errorControl;
			ustring binaryPathName;
			ustring loadOrderGroup;
			DWORD tagId;
			Base::mstring dependencies;
			ustring serviceStartName;
			ustring description;
			SERVICE_STATUS_PROCESS status;

			Info_t(SC_HANDLE scm, const ENUM_SERVICE_STATUS_PROCESSW & st);

			Info_t(PCWSTR _name, const Service & svc);

			bool operator < (const Info_t & rhs) const;

			bool operator == (const ustring & nm) const;

			Type_t get_type() const {
				return (Type_t)status.dwServiceType;
			}

			State_t get_state() const {
				return (State_t)status.dwCurrentState;
			}

			bool is_service() const {
				return static_cast<DWORD>(get_type()) & (SERVICE_WIN32_OWN_PROCESS | SERVICE_WIN32_SHARE_PROCESS);
			}

			bool is_disabled() const {
				return startType == Start_t::DISABLED;
			}
		};


		///================================================================================= Manager
		struct Manager: Base::Uncopyable {
			~Manager();

			Manager(RemoteConnection * conn = nullptr, ACCESS_MASK acc = SC_MANAGER_CONNECT);

			Manager(Manager && right);

			Manager & operator = (Manager && right);

			operator SC_HANDLE() const {
				return m_hndl;
			}

			void reconnect(RemoteConnection * conn = nullptr, ACCESS_MASK acc = SC_MANAGER_CONNECT);

			Service create_service(const Service::Create_t & info) const;

			bool is_exist(PCWSTR name) const;

		private:
			static SC_HANDLE open(RemoteConnection * conn, ACCESS_MASK acc);
			static void close(SC_HANDLE scm);

			SC_HANDLE m_hndl;
		};


		///================================================================================= Service
		~Service();

		Service(SC_HANDLE scm, PCWSTR name, ACCESS_MASK access = SERVICE_QUERY_CONFIG);

		Service(Service && right);

		Service & operator = (Service && right);

		operator SC_HANDLE() const {
			return m_hndl;
		}

		void del();

		Service & start();
		Service & stop();
		Service & restart();
		Service & contin();
		Service & pause();

		Service & set_config(const Service::Config_t & info);
		Service & set_logon(const Service::Logon_t & info);
		Service & set_description(PCWSTR info);
		Service & set_delayed(bool state);

		Service & wait_state(State_t state, DWORD dwTimeout);

		ustring get_description() const;
		bool get_delayed() const;
		Service::Status_t get_status() const;
		Service::State_t get_state() const;
		Service::Start_t get_start_type() const;
		DWORD get_type() const;
		ustring get_user() const;

		static void del(SC_HANDLE scm, PCWSTR name);
		static Service start(SC_HANDLE scm, PCWSTR name);
		static Service stop(SC_HANDLE scm, PCWSTR name);
		static Service restart(SC_HANDLE scm, PCWSTR name);
		static Service contin(SC_HANDLE scm, PCWSTR name);
		static Service pause(SC_HANDLE scm, PCWSTR name);

		static Service set_config(SC_HANDLE scm, PCWSTR name, const Service::Config_t & info);
		static Service set_logon(SC_HANDLE scm, PCWSTR name, const Service::Logon_t & info);
		static Service set_desription(SC_HANDLE scm, PCWSTR name, PCWSTR info);

		static Service::Status_t get_status(SC_HANDLE scm, PCWSTR name);
		static Service::State_t get_state(SC_HANDLE scm, PCWSTR name);
		static Service::Start_t get_start_type(SC_HANDLE scm, PCWSTR name);
		static ustring get_description(SC_HANDLE scm, PCWSTR name);

	private:
		Service(SC_HANDLE svc):
			m_hndl(svc)
		{
		}

		Base::auto_buf<LPQUERY_SERVICE_CONFIGW> QueryConfig() const;
		Base::auto_buf<PBYTE> QueryConfig2(DWORD level) const;

		SC_HANDLE m_hndl;

		friend struct Manager;
		friend struct Info_t;
	};

}

#endif
