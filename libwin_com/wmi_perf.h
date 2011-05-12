#ifndef _WIN_WMI_PERF_H_
#define _WIN_WMI_PERF_H_

#include "wmi.h"

///==================================================================================== WmiRefresher
class		WmiRefresher {
	const WmiConnection		&m_conn;
	ComObject<IWbemRefresher> m_refr;
	ComObject<IWbemHiPerfEnum> m_enum;

public:
	WmiRefresher(const WmiConnection &conn, PCWSTR classname);

	void		Refresh() const {
		m_refr->Refresh(0L);
	}

	IWbemHiPerfEnum*	operator->() const {
		return	m_enum;
	}
};

///========================================================================================= WMIPerf
class		WMIPerf {
	WmiRefresher*	m_refr[2];
public:
	~WMIPerf() {
		delete m_refr[0];
		delete m_refr[1];
	}
	WMIPerf(const WmiConnection &conn, PCWSTR in) {
		m_refr[0] = new WmiRefresher(conn, in);
		m_refr[1] = new WmiRefresher(conn, in);
	}
	void		Refresh() {
		std::swap(m_refr[0], m_refr[1]);
		m_refr[1]->Refresh();
	}
};

///================================================================================== WmiPerfObjects
class		WmiPerfObjects {
	const WmiRefresher &m_refr;
	IWbemObjectAccess	**apEnumAccess;
	DWORD				m_cnt;

	void	Read();
	void	Close();
public:
	~WmiPerfObjects() {
		Close();
	}
	WmiPerfObjects(const WmiRefresher &refr):
		m_refr(refr),
		apEnumAccess(nullptr),
		m_cnt(0) {
		Refresh();
	}

	void		Refresh() {
		m_refr.Refresh();
		Read();
	}

	size_t		count() const {
		return	m_cnt;
	}

	uint32_t	get_dword(size_t index, PCWSTR name) const;
	uint64_t	get_qword(size_t index, PCWSTR name) const;
	AutoUTF		get_str(size_t index, PCWSTR name) const;
};

#endif
