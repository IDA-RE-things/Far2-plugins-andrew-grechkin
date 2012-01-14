#include "farplugin.hpp"

ustring make_path(const ustring & path, const ustring & name) {
	return path + PATH_SEPARATOR + name;
}

Options::Options() {
	op = 0;
}

void Options::load() {
#ifndef FAR2
	m_settings.reset(new Far::Settings_t(FarPlugin::get_guid()));
	m_whitespaces = m_settings->get(L"whitespaces", L" ");
#else
	m_settings.reset(new Register);
	m_settings->open_key(KEY_READ | KEY_WRITE, make_path(Far::get_plugin_path(), plugin->get_name()).c_str());
	WCHAR whsp[32];
	m_settings->get(L"whitespace", whsp, sizeof(whsp));
	m_whitespaces = whsp;
#endif
	inv = m_settings->get(L"invert", 0);
	cs = m_settings->get(L"case", 0);
	ns = m_settings->get(L"numeric", 0);
	sel = m_settings->get(L"selection", 0);
	emp = m_settings->get(L"asempty", 0);
}

void Options::get_parameters(const Far::Dialog & dlg) {
	inv = dlg.Check(indInv);
	cs = dlg.Check(indCS);
	ns = dlg.Check(indNS);
	sel = dlg.Check(indSelected);
	emp = dlg.Check(indAsEmpty);
	op = dlg.get_list_position(indList);
	m_whitespaces = dlg.Str(indWhsp);
}

void Options::save() const {
	m_settings->set(L"invert", inv);
	m_settings->set(L"case", cs);
	m_settings->set(L"numeric", ns);
	m_settings->set(L"selection", sel);
	m_settings->set(L"asempty", emp);
	m_settings->set(L"whitespace", get_whitespaces().c_str());
}

bool Options::is_whitespace(WCHAR ch) const {
	return Find(m_whitespaces.c_str(), ch);
}

ustring Options::get_whitespaces() const {
	return m_whitespaces;
}

size_t Options::get_first_line() const {
	return m_first_line;
}

size_t Options::get_total_lines() const {
	return m_ei.TotalLines;
}

ssize_t Options::get_block_type() const {
	return m_ei.BlockType;
}

void Options::load_editor_info() {
	Far::Editor::get_info(m_ei);
	if (m_ei.BlockType == BTYPE_STREAM || m_ei.BlockType == BTYPE_COLUMN)
		m_first_line = m_ei.BlockStartLine;
	else
		m_first_line = 0;
}
