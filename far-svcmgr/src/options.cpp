#include "farplugin.hpp"

ustring make_path(const ustring & path, const ustring & name) {
	return path + PATH_SEPARATOR + name;
}

Options::Options() {
	AddToPluginsMenu = 1;
	AddToDisksMenu = 0;
	TimeOut = 0;
	Copy(Prefix, L"svcmgr");
	Timeout[0] = L'\0';
}

void Options::load() {
	m_settings.reset(new Far::Settings_t(FarPlugin::get_guid()));
//	op = m_settings->get(L"operation", 1);
//	opm = m_settings->get(L"is_text_mask", 0);
}

void Options::get_parameters(const Far::Dialog & /*dlg*/) {
//	opm = dlg.Check(indIsMask);
}

void Options::save() const {
//	m_settings->set(L"operation", op);
//	m_settings->set(L"is_text_mask", opm);

}
