#include "wmi_perf.h"
#include <libwin_net/exception.h>

///==================================================================================== WmiRefresher
WmiRefresher::WmiRefresher(const WmiConnection &conn, PCWSTR /*classname*/):
	m_conn(conn) {
	CheckCom(::CoCreateInstance(CLSID_WbemRefresher, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (PVOID*)&m_refr));
	ComObject<IWbemConfigureRefresher> m_confrefr;
	CheckCom(m_refr->QueryInterface(IID_IWbemConfigureRefresher, (PVOID*)&m_confrefr));
//	long lID = 0;
//	CheckCom(m_confrefr->AddEnum(m_conn.GetIWbemServices(), classname, 0, nullptr, &m_enum, &lID));
}

class AccessLock {
public:
	~AccessLock() {
		m_acc->Unlock(0);
	}
	AccessLock(IWbemObjectAccess *acc):
		m_acc(acc) {
		m_acc->Lock(0);
	}
private:
	IWbemObjectAccess *m_acc;
};

///================================================================================== WmiPerfObjects
void		WmiPerfObjects::Close() {
	for (size_t i = 0; i < m_cnt; ++i) {
		if (apEnumAccess[i]) {
			apEnumAccess[i]->Release();
			apEnumAccess[i] = NULL;
		}
	}
	WinMem::Free(apEnumAccess);
	m_cnt = 0;
}

void		WmiPerfObjects::Read() {
	Close();
	DWORD size = 0;
	DWORD err = m_refr->GetObjects(0L, m_cnt, apEnumAccess, &size);
	if (size && err == WBEM_E_BUFFER_TOO_SMALL) {
		apEnumAccess = (IWbemObjectAccess**)WinMem::Alloc(size * sizeof(PVOID), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY);
		m_cnt = size;
		CheckWmi(m_refr->GetObjects(0L, m_cnt, apEnumAccess, &size));
	} else {
		CheckWmi(err);
	}
}

uint32_t	WmiPerfObjects::get_dword(size_t index, PCWSTR name) const {
	if (index >= m_cnt)
		CheckWmi(WBEM_E_VALUE_OUT_OF_RANGE);
	long	hnd = 0;
	AccessLock lock(apEnumAccess[index]);
	CheckWmi(apEnumAccess[index]->GetPropertyHandle(name, nullptr, &hnd));
	DWORD ret;
	CheckWmi(apEnumAccess[index]->ReadDWORD(hnd, &ret));
	return ret;
}

uint64_t	WmiPerfObjects::get_qword(size_t index, PCWSTR name) const {
	if (index >= m_cnt)
		CheckWmi(WBEM_E_VALUE_OUT_OF_RANGE);
	long	hnd = 0;
	AccessLock lock(apEnumAccess[index]);
	CheckWmi(apEnumAccess[index]->GetPropertyHandle(name, nullptr, &hnd));
	uint64_t ret;
	CheckWmi(apEnumAccess[index]->ReadQWORD(hnd, &ret));
	return ret;
}

ustring		WmiPerfObjects::get_str(size_t index, PCWSTR name) const {
	if (index >= m_cnt)
		CheckWmi(WBEM_E_VALUE_OUT_OF_RANGE);
	long	hnd = 0;
	AccessLock lock(apEnumAccess[index]);
	CheckWmi(apEnumAccess[index]->GetPropertyHandle(name, nullptr, &hnd));
	long	size = 0;
	DWORD hr = apEnumAccess[index]->ReadPropertyValue(hnd, 0, &size, nullptr);
	if (FAILED(hr)) {
		if (hr == WBEM_E_BUFFER_TOO_SMALL) {
			auto_buf<PWSTR> buf(size);
			CheckWmi(apEnumAccess[index]->ReadPropertyValue(hnd, buf.size(), &size, (PBYTE)buf.data()));
			return ustring(buf.data());
		} else {
			CheckWmi(hr);
		}
	}
	return ustring();
}
