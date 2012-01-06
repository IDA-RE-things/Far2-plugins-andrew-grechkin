#include "farplugin.hpp"

Options::Options() {
	op = 0;
}

#ifndef FAR2
void Options::load() {
	m_settings.reset(new Far::Settings_t(*Far::guid()));
	inv = m_settings->get(FSSF_ROOT, L"invert", 0);
	cs = m_settings->get(FSSF_ROOT, L"case", 0);
	ns = m_settings->get(FSSF_ROOT, L"numeric", 0);
	sel = m_settings->get(FSSF_ROOT, L"selection", 0);
	emp = m_settings->get(FSSF_ROOT, L"asempty", 0);
	m_whitespaces = m_settings->get(FSSF_ROOT, L"whitespaces", L" ");
}
#else
void Options::load(const ustring & path) {
	reg.open_key(KEY_READ | KEY_WRITE, path.c_str());

	inv = reg.get(L"invert", 0);
	cs = reg.get(L"case", 0);
	ns = reg.get(L"numeric", 0);
	sel = reg.get(L"selection", 0);
	emp = reg.get(L"asempty", 0);
	WCHAR whsp[32];
	reg.get(L"whitespace", whsp, sizeof(whsp));
	m_whitespaces = whsp;
}
#endif

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
#ifndef FAR2
	m_settings->set(FSSF_ROOT, L"invert", inv);
	m_settings->set(FSSF_ROOT, L"case", cs);
	m_settings->set(FSSF_ROOT, L"numeric", ns);
	m_settings->set(FSSF_ROOT, L"selection", sel);
	m_settings->set(FSSF_ROOT, L"asempty", emp);
	m_settings->set(FSSF_ROOT, L"whitespace", get_whitespaces().c_str());
#else
	reg.set(L"invert", inv);
	reg.set(L"case", cs);
	reg.set(L"numeric", ns);
	reg.set(L"selection", sel);
	reg.set(L"asempty", emp);
	reg.set(L"whitespace", get_whitespaces().c_str());
#endif
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
