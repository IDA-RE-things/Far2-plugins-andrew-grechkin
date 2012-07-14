#include <libcom/shlink.hpp>
#include <libext/exception.hpp>
#include <libbase/str.hpp>

#include <shlguid.h>
#include <Shobjidl.h>

ShellLink::~ShellLink() {
}

ShellLink::ShellLink() {
}

ShellLink::ShellLink(PCWSTR path, bool write):
	m_path(path) {
	WinCOM::init();
	ComObject<IPersistFile> ppf;
	CheckApiError(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
	                                 IID_IPersistFile, (PVOID*)&ppf));
	CheckCom(ppf->Load(m_path.c_str(), write ? STGM_READWRITE : STGM_READ));

	CheckCom(ppf->QueryInterface(IID_IShellLink, (PVOID*)&m_lnk));

	CheckCom(m_lnk->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI));
}

ShellLink::ShellLink(const ShellLink &rhs):
	m_path(rhs.m_path),
	m_lnk(rhs.m_lnk) {
}

ShellLink& ShellLink::operator=(const ShellLink &rhs) {
	m_path = rhs.m_path;
	m_lnk = rhs.m_lnk;
	return *this;
}

ustring ShellLink::args() const {
	Base::auto_array<WCHAR> buf(Base::MAX_PATH_LEN);
	CheckCom(m_lnk->GetArguments(buf.data(), buf.size()));
	return ustring(buf);
}

ustring ShellLink::descr() const {
	Base::auto_array<WCHAR> buf(Base::MAX_PATH_LEN);
	CheckCom(m_lnk->GetDescription(buf.data(), buf.size()));
	return ustring(buf);
}

ShellIcon ShellLink::icon() const {
	Base::auto_array<WCHAR> buf(Base::MAX_PATH_LEN);
	int ind;
	CheckCom(m_lnk->GetIconLocation(buf.data(), buf.size(), &ind));

	return ShellIcon(ustring(buf.data()), ind);
}

ustring ShellLink::path() const {
	Base::auto_array<WCHAR> buf(Base::MAX_PATH_LEN);
	CheckCom(m_lnk->GetPath(buf.data(), buf.size(), nullptr, 0));
	return ustring(buf);
}

int ShellLink::show() const {
	int ret;
	CheckCom(m_lnk->GetShowCmd(&ret));
	return ret;
}

ustring ShellLink::work_dir() const {
	Base::auto_array<WCHAR> buf(Base::MAX_PATH_LEN);
	CheckCom(m_lnk->GetWorkingDirectory(buf.data(), buf.size()));
	return ustring(buf.data());
}

void ShellLink::args(const ustring &in) {
	CheckCom(m_lnk->SetArguments(in.c_str()));
}

void ShellLink::descr(const ustring &in) {
	CheckCom(m_lnk->SetDescription(in.c_str()));
}

void ShellLink::path(const ustring &in) {
	CheckCom(m_lnk->SetPath(in.c_str()));
}

void ShellLink::show(int in) {
	CheckCom(m_lnk->SetShowCmd(in));
}

void ShellLink::work_dir(const ustring &in) {
	CheckCom(m_lnk->SetWorkingDirectory(in.c_str()));
}

void ShellLink::write() const {
	ComObject<IPersistFile> ppf;
	CheckCom(m_lnk->QueryInterface(IID_IPersistFile, (PVOID*)&ppf));
	CheckCom(ppf->Save(m_path.c_str(), FALSE));
}

ShellLink ShellLink::create(PCWSTR path) {
	ShellLink ret;
	ret.m_path = path;
	CheckCom(::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (PVOID*)&ret.m_lnk));
	return ret;
}
