#include <libext/sd.hpp>
#include <libext/exception.hpp>
#include <libext/dacl.hpp>
#include <libbase/logger.hpp>

namespace Ext {

	///==================================================================================== WinAbsSD
	void WinAbsSD::Init(PSECURITY_DESCRIPTOR sd) {
		m_owner = m_group = m_dacl = m_sacl = nullptr;
		DWORD sdSize = SECURITY_DESCRIPTOR_MIN_LENGTH;
		DWORD ownerSize = SECURITY_MAX_SID_SIZE;
		DWORD groupSize = SECURITY_MAX_SID_SIZE;
		DWORD daclSize = WinSD::get_dacl_size(sd);
		DWORD saclSize = WinSD::get_sacl_size(sd);
		m_sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, sdSize);
		m_owner = (PSID)::LocalAlloc(LPTR, ownerSize);
		m_group = (PSID)::LocalAlloc(LPTR, groupSize);
		if (daclSize)
			m_dacl = (PACL)::LocalAlloc(LPTR, daclSize);
		if (saclSize)
			m_sacl = (PACL)::LocalAlloc(LPTR, saclSize);
		CheckApi(::MakeAbsoluteSD(sd, m_sd, &sdSize, m_dacl, &daclSize,
		                          m_sacl, &saclSize, m_owner, &ownerSize, m_group, &groupSize));
	}

	WinAbsSD::~WinAbsSD() {
		LogTrace();
		if (m_owner)
			::LocalFree(m_owner);
		if (m_group)
			::LocalFree(m_group);
		if (m_dacl)
			::LocalFree(m_dacl);
		if (m_sacl)
			::LocalFree(m_sacl);
	}

	WinAbsSD::WinAbsSD():
		m_owner(nullptr),
		m_group(nullptr),
		m_dacl(nullptr),
		m_sacl(nullptr)
	{
		LogTrace();
		m_sd = alloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));
	}

	WinAbsSD::WinAbsSD(const ustring & name, const ustring & group, bool prot):
		m_owner(nullptr),
		m_group(nullptr),
		m_dacl(nullptr),
		m_sacl(nullptr)
	{
		m_sd = alloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));
		m_dacl = WinDacl::create(64);

		if (!name.empty()) {
			try {
				Sid usr(name);
				DWORD ownerSize = SECURITY_MAX_SID_SIZE;
				m_owner = (PSID)::LocalAlloc(LPTR, ownerSize);
				usr.copy_to(m_owner, ownerSize);
			} catch (...) {
			}
		}

		if (!group.empty()) {
			try {
				DWORD groupSize = SECURITY_MAX_SID_SIZE;
				m_group = (PSID)::LocalAlloc(LPTR, groupSize);
				Sid grp(group);
				grp.copy_to(m_group, groupSize);
			} catch (...) {
			}
		}

		set_owner(m_sd, m_owner);
		set_group(m_sd, m_group);
		set_dacl(m_sd, m_dacl);
		CheckApi(::IsValidSecurityDescriptor(m_sd));
		set_protect(prot);
	}

	WinAbsSD::WinAbsSD(PSID owner, PSID group, PACL dacl, bool protect):
		m_owner(nullptr),
		m_group(nullptr),
		m_dacl(nullptr),
		m_sacl(nullptr)
	{
		m_sd = alloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		CheckApi(::InitializeSecurityDescriptor(m_sd, SECURITY_DESCRIPTOR_REVISION));

		try {
			m_owner = Sid::clone(owner);
			set_owner(m_sd, m_owner);
		} catch (AbstractError & e) {
			LogDebug(L"exception cought: %s, %s", e.what().c_str(), e.where());
		}

		try {
			m_group = Sid::clone(group);
			set_group(m_sd, m_group);
		} catch (AbstractError & e) {
			LogDebug(L"exception cought: %s, %s", e.what().c_str(), e.where());
		}

		try {
			WinDacl(dacl).detach(m_dacl);
			set_dacl(m_sd, m_dacl);
		} catch (AbstractError & e) {
			LogDebug(L"exception cought: %s, %s", e.what().c_str(), e.where());
		}

		CheckApi(::IsValidSecurityDescriptor(m_sd));
		set_protect(protect);
	}

}
