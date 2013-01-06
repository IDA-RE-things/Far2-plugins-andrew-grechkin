#ifndef _LIBCOM_QUOTA_HPP_
#define _LIBCOM_QUOTA_HPP_

#include <libbase/std.hpp>

#include <map>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ com_quota
struct IDiskQuotaControl;
struct IDiskQuotaUser;
struct IEnumDiskQuotaUsers;

namespace Com {
	///=================================================================================== QuotaInfo
	struct QuotaInfo {
		QuotaInfo(const ComObject<IDiskQuotaControl> & ctrl, PCWSTR name);
		QuotaInfo(const ComObject<IDiskQuotaUser> & usr);
		QuotaInfo(const ComObject<IDiskQuotaUser> & usr, const ustring & name);

		ustring get_name() const;
		ustring get_used_text() const;
		ustring get_limit_text() const;
		ustring get_threshold_text() const;

		size_t get_used() const;
		size_t get_limit() const;
		size_t get_threshold() const;

		void set_limit(size_t in);
		void set_threshold(size_t in);

	private:
		static ustring get_name(const ComObject<IDiskQuotaUser> &usr);

		ComObject<IDiskQuotaUser> m_usr;
		mutable ustring m_name;

		friend class DiskQuotaUsers;
	};

	///=================================================================================== DiskQuota
	struct DiskQuota {
		enum DiskQuotaState {
			DISABLED,
			TRACKED,
			ENABLED,
		};

		static bool is_supported(const ustring &path);

		~DiskQuota();

		DiskQuota(const ustring &path);

		const ComObject<IDiskQuotaControl>& operator->() const;

		ustring path() const;

		//change state
		void set_state(DiskQuotaState state) const;

		DiskQuotaState get_state() const;

		//check state
		bool is_log_limit() const;

		bool is_log_threshold() const;

		//change limits
		void set_default_limit(size_t in = 0) const;

		void set_default_threshold(size_t in = 0) const;

		void set_log_limit(bool in = true) const;

		void set_log_threshold(bool in = true) const;

		size_t get_default_limit() const;

		size_t get_default_threshold() const;

		ustring get_default_limit_text() const;

		ustring get_default_threshold_text() const;

		ustring parse_state() const;

		void add(const ustring & name);

		void add(const ustring & name, size_t lim, size_t thr);

		void del(const ustring & name);

	private:
		void set_state_raw(DWORD in) const;

		DWORD local_state_to_raw(DiskQuotaState state) const;

		DiskQuotaState raw_state_to_local(DWORD in) const;

		DWORD get_state_raw() const;

		DWORD get_log_flags() const;

		ComObject<IDiskQuotaUser> get_user_raw(const ustring & name) const;

		ComObject<IDiskQuotaUser> add_raw(const ustring & name);

		ComObject<IDiskQuotaUser> add_raw(const ustring & name, size_t lim, size_t thr);

		void del(const ComObject<IDiskQuotaUser> & user);

		ustring m_path;
		ComObject<IDiskQuotaControl> m_control;

		friend class DiskQuotaUsers;
	};

	///============================================================================== DiskQuotaUsers
	struct DiskQuotaUsers: private std::multimap<ustring, QuotaInfo> {
		DiskQuotaUsers(const DiskQuota & nq);

		void cache();

		void add(const ustring &name, size_t lim = 0, size_t thr = 0);

		void del(const ustring &name);

	private:
		DiskQuota m_nq;
	};

}

#endif
