#include "win_net.h"

#include <iphlpapi.h>

///=========================================================================================== WinIf
bool	WinIp::Cache() {
	ULONG	size = 0;
	if (::GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &size) == ERROR_BUFFER_OVERFLOW) {
		auto_buf<PIP_ADAPTER_ADDRESSES> ipbuf(size);
		if (::GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, ipbuf, &size) == ERROR_SUCCESS) {
			Clear();
			PIP_ADAPTER_ADDRESSES pCurrAddresses = ipbuf;
			while (pCurrAddresses) {
				size_t	i = 0;
				PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
				for (i = 0; pUnicast != nullptr; ++i) {
					Insert(IpAsStr(pUnicast->Address), pUnicast->Address);
					pUnicast = pUnicast->Next;
				}
				pCurrAddresses = pCurrAddresses->Next;
			}
			return	true;
		}
	}
	return	false;
}
