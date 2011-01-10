﻿/**
	win_ts

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_TS_HPP
#define WIN_TS_HPP

#include "win_net.h"

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ net_ts
///===================================================================================== WinTSHandle
struct		WinTSHandle: private Uncopyable {
	~WinTSHandle();

	WinTSHandle();

	WinTSHandle(PCWSTR host);

	operator		HANDLE() const {
		return	m_ts;
	}

private:
	HANDLE		m_ts;
};

///===================================================================================== WinTSession
namespace	WinTSession {
	void	ConnectLocal(DWORD id, PCWSTR pass = L"");

	void	ConnectRemote(DWORD id, PCWSTR host);

	void	Disconnect(DWORD id, const WinTSHandle &host = WinTSHandle());

	void	LogOff(DWORD id, const WinTSHandle &host = WinTSHandle());

	DWORD	Question(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, const WinTSHandle &host = WinTSHandle());

	DWORD	Message(DWORD id, PCWSTR ttl, PCWSTR msg, DWORD time = 60, bool wait = true, const WinTSHandle &host = WinTSHandle());

	void	Reboot(const WinTSHandle &host = WinTSHandle());

	void	Turnoff(const WinTSHandle &host = WinTSHandle());
};

///======================================================================================= WinTSInfo
class	WinTSInfo {
public:
	WinTSInfo(DWORD i, const ustring &s, const ustring &u, int st):
		m_impl(new impl(i, s, u, st)) {
	}

	PCWSTR	GetState() const {
		return	ParseState(m_impl->state);
	}

	PCWSTR	ParseState(int st) const;

	PCWSTR	ParseStateFull(int st) const;

	ustring	Info() const;

	void winSta(const ustring &in) {
		m_impl->winSta = in;
	}

	void client(const ustring &in) {
		m_impl->client = in;
	}

	DWORD id() const {
		return m_impl->id;
	}
	ustring	sess() const {
		return m_impl->sess;
	}
	ustring	user() const {
		return m_impl->user;
	}
	ustring	winSta() const {
		return m_impl->winSta;
	}
	ustring	client() const {
		return m_impl->client;
	}
	int		state() const {
		return m_impl->state;
	}

	bool is_disconnected() const;

	operator DWORD() const {
		return m_impl->id;
	}

private:
	struct impl {
		DWORD	id;
		ustring	sess;
		ustring	user;
		ustring	winSta;
		ustring	client;
		int		state;

		impl(DWORD i, const ustring &s, const ustring &u, int st):
			id(i),
			sess(s),
			user(u),
			state(st) {
		}
	};

	winstd::shared_ptr<impl> m_impl;
};

template<typename Type>
class VectorContainer {
	template<typename Type1>
	friend bool operator==(const VectorContainer<Type1>&, const VectorContainer<Type1>&);

	template<typename Type1>
	friend bool operator<(const VectorContainer<Type1>&, const VectorContainer<Type1>&);

public:
	typedef std::vector<Type> container_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef typename std::pair<iterator, iterator> pair_iterator;

public:
	void clear() {
		m_c.clear();
	}

	bool empty() const {
		return m_c.empty();
	}

	size_type size() const {
		return m_c.size();
	}

	iterator begin() {
		return m_c.begin();
	}

	const_iterator begin() const {
		return m_c.begin();
	}

	iterator end() {
		return m_c.end();
	}

	const_iterator end() const {
		return m_c.end();
	}

	template<typename Type2>
	iterator find(const Type2 &val) {
		pair_iterator p = std::equal_range(begin(), end(), val);
		if (p.first != p.second)
			return p.first;
		return end();
//		return std::find(begin(), end(), val);
	}

	template<typename Type2>
	bool exist(const Type2 &name) {
		return find(name) != end();
	}
protected:
	VectorContainer() {
	}
	container_type m_c;
};

///==================================================================================== WinTSessions
class	WinTS : public VectorContainer<WinTSInfo> {
public:
	void Cache(const WinTSHandle &host);

	bool	FindSess(PCWSTR in) const;
	bool	FindUser(PCWSTR in) const;
};

#endif // WIN_TS_HPP