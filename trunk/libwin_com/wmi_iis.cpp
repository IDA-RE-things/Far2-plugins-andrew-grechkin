#include "wmi_iis.h"

///==================================================================================== WmiIisServer
AutoUTF WmiIisServer::name() const {
	return get_param(L"Name").as_str();
}

BStr WmiIisServer::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Server.Name=\"%s\"", name);
	return BStr(path);
}

///====================================================================================== WmiIisSite
AutoUTF WmiIisSite::name() const {
	return get_param(L"Name").as_str();
}

size_t WmiIisSite::id() const {
	return get_param(L"Id").as_int();
}

bool WmiIisSite::is_enabled() const {
	return get_param(L"ServerAutoStart").as_bool();
}

void WmiIisSite::enable() {
	Variant tmp(true);
	CheckWmi(m_obj->Put(L"ServerAutoStart", 0, &tmp, 0));
	conn().update(m_obj);
}

void WmiIisSite::disable() {
	Variant tmp(false);
	CheckWmi(m_obj->Put(L"ServerAutoStart", 0, &tmp, 0));
	conn().update(m_obj);
}

BStr WmiIisSite::Path(PCWSTR name) const {
	WCHAR	path[MAX_PATH];
	::_snwprintf(path, sizeofa(path), L"Site.Name=\"%s\"", name);
	return BStr(path);
}

