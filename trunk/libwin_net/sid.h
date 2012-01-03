#ifndef WIN_NET_SID_HPP
#define WIN_NET_SID_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_sid
///============================================================================================= Sid
/// Security Identifier (Идентификатор безопасности) -
/// структура данных переменной длины, которая идентифицирует учетную запись пользователя, группы,
/// домена или компьютера
#ifndef PSIDFromPACE
#define PSIDFromPACE(pACE)((PSID)(&((pACE)->SidStart)))
#endif

class Sid {
	typedef Sid class_type;

public:
	typedef PSID value_type;
	typedef size_t size_type;

	~Sid();

	explicit Sid(WELL_KNOWN_SID_TYPE wns);

	explicit Sid(value_type rhs);

	Sid(const class_type &rhs);

	explicit Sid(PCWSTR name, PCWSTR srv = nullptr);

	explicit Sid(const ustring &name, PCWSTR srv = nullptr);

	class_type& operator=(value_type rhs);

	class_type& operator=(const class_type &rhs);

	bool operator==(value_type rhs) const;

	bool operator==(const class_type &rhs) const {
		return operator==(rhs.m_sid);
	}

	bool operator!=(value_type rhs) const {
		return !operator==(rhs);
	}

	bool operator!=(const class_type &rhs) const {
		return !operator==(rhs.m_sid);
	}

	size_type size() const {
		return class_type::size(m_sid);
	}

	bool is_valid() const {
		return class_type::is_valid(m_sid);
	}

	ustring str() const {
		return class_type::str(m_sid);
	}

	ustring name() const {
		return class_type::name(m_sid);
	}
	ustring full_name() const {
		return class_type::full_name(m_sid);
	}
	ustring domain() const {
		return class_type::domain(m_sid);
	}

	void copy_to(value_type out, size_t size) const;

	operator value_type() const {
		return m_sid;
	}

	void detach(value_type &sid);

	void swap(class_type &rhs);

	static bool is_valid(value_type in) {
		return in && ::IsValidSid(in);
	}
	static void check(value_type in);
	static size_type size(value_type in);
	static size_type sub_authority_count(value_type in);
	static size_type rid(value_type in);

	// PSID to sid string
	static ustring str(value_type in);

	// name to sid string
	static ustring str(const ustring &name, PCWSTR srv = nullptr);

	// PSID to name
	static void name(value_type pSID, ustring &name, ustring &dom, PCWSTR srv = nullptr);
	static ustring name(value_type pSID, PCWSTR srv = nullptr);
	static ustring full_name(value_type pSID, PCWSTR srv = nullptr);
	static ustring domain(value_type pSID, PCWSTR srv = nullptr);

protected:
	Sid(): m_sid(nullptr) {
	}

	value_type m_sid;

private:
	void init(value_type in);
	void init(PCWSTR name, PCWSTR srv = nullptr);
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

ustring	get_token_user(HANDLE hToken);

#endif
