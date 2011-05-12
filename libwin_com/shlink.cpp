#include "shlink.h"

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

AutoUTF ShellLink::args() const {
	auto_array<WCHAR> buf(MAX_PATH_LEN);
	CheckCom(m_lnk->GetArguments(buf, buf.size()));
	return AutoUTF(buf);
}

AutoUTF ShellLink::descr() const {
	auto_array<WCHAR> buf(MAX_PATH_LEN);
	CheckCom(m_lnk->GetDescription(buf, buf.size()));
	return AutoUTF(buf);
}

ShellIcon ShellLink::icon() const {
	auto_array<WCHAR> buf(MAX_PATH_LEN);
	int ind;
	CheckCom(m_lnk->GetIconLocation(buf, buf.size(), &ind));

	return ShellIcon(AutoUTF(buf), ind);
}

AutoUTF ShellLink::path() const {
	auto_array<WCHAR> buf(MAX_PATH_LEN);
	CheckCom(m_lnk->GetPath(buf, buf.size(), nullptr, 0));
	return AutoUTF(buf);
}

int ShellLink::show() const {
	int ret;
	CheckCom(m_lnk->GetShowCmd(&ret));
	return ret;
}

AutoUTF ShellLink::work_dir() const {
	auto_array<WCHAR> buf(MAX_PATH_LEN);
	CheckCom(m_lnk->GetWorkingDirectory(buf, buf.size()));
	return AutoUTF(buf);
}

void ShellLink::args(const AutoUTF &in) {
	CheckCom(m_lnk->SetArguments(in.c_str()));
}

void ShellLink::descr(const AutoUTF &in) {
	CheckCom(m_lnk->SetDescription(in.c_str()));
}

void ShellLink::path(const AutoUTF &in) {
	CheckCom(m_lnk->SetPath(in.c_str()));
}

void ShellLink::show(int in) {
	CheckCom(m_lnk->SetShowCmd(in));
}

void ShellLink::work_dir(const AutoUTF &in) {
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
