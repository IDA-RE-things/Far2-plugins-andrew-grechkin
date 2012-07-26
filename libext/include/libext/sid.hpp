#ifndef WIN_NET_SID_HPP
#define WIN_NET_SID_HPP

#include <libbase/std.hpp>

namespace Ext {

#ifndef PSIDFromPACE
#define PSIDFromPACE(pACE) ((PSID)(&((pACE)->SidStart)))
#endif

	///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sid
	///============================================================================================= Sid
	/// Security Identifier (Идентификатор безопасности) -
	/// структура данных переменной длины, которая идентифицирует учетную запись пользователя, группы,
	/// домена или компьютера
	struct Sid {
		typedef Sid this_type;
		typedef PSID value_type;
		typedef size_t size_type;

		~Sid();

		explicit Sid(WELL_KNOWN_SID_TYPE wns);

		explicit Sid(value_type rhs);

		explicit Sid(PCWSTR name, PCWSTR srv = nullptr);

		explicit Sid(const ustring & name, PCWSTR srv = nullptr);

		Sid(const this_type & rhs);

		Sid(this_type && rhs);

		this_type & operator = (const this_type & rhs);

		this_type & operator = (this_type && rhs);

		this_type & operator = (value_type rhs);

		bool operator == (value_type rhs) const;

		bool operator == (const this_type & rhs) const {
			return operator == (rhs.m_sid);
		}

		bool operator != (value_type rhs) const {
			return !operator == (rhs);
		}

		bool operator != (const this_type & rhs) const {
			return !operator == (rhs.m_sid);
		}

		size_type size() const;

		bool is_valid() const;

		ustring as_str() const;

		ustring get_name() const;
		ustring get_full_name() const;
		ustring get_domain() const;

		void copy_to(value_type out, size_t size) const;

		operator value_type() const {
			return m_sid;
		}

		void detach(value_type & sid);

		void swap(this_type & rhs);

		static bool is_valid(value_type in);
		static void check(value_type in);
		static size_type size(value_type in);
		static size_type sub_authority_count(value_type in);
		static size_type get_rid(value_type in);

		// PSID to sid string
		static ustring as_str(value_type in);

		// PSID to name
		static void get_name_dom(value_type sid, ustring & name, ustring & dom, PCWSTR srv = nullptr);
		static ustring get_name(value_type sid, PCWSTR srv = nullptr);
		static ustring get_full_name(value_type sid, PCWSTR srv = nullptr);
		static ustring get_domain(value_type sid, PCWSTR srv = nullptr);

		static PSID clone(value_type in);
		static PSID get_sid(WELL_KNOWN_SID_TYPE wns);
		static PSID get_sid(PCWSTR name, PCWSTR srv = nullptr);

	protected:
		Sid() :
			m_sid(nullptr) {
		}

		value_type m_sid;
	};

	struct SidString: public Sid {
		explicit SidString(PCWSTR str) {
			init(str);
		}

		explicit SidString(const ustring & str) {
			init(str.c_str());
		}

	private:
		void init(PCWSTR str);
	};

	bool is_admin();

	ustring get_token_user(HANDLE hToken);

}

#endif
