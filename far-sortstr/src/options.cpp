#include "farplugin.hpp"

Options::Options():
	m_whitespaces(L" ") {
}

#ifndef FAR2
void Options::load(const ustring & /*path*/) {
}
#else
void Options::load(const ustring & path) {
	reg.Open(KEY_READ | KEY_WRITE, path.c_str());

	reg.Get(L"invert", inv, 0);
	reg.Get(L"case", cs, 0);
	reg.Get(L"numeric", ns, 0);
	reg.Get(L"selection", sel, 0);
	reg.Get(L"asempty", emp, 0);
	WCHAR whsp[32];
	reg.GetStr(L"whitespace", whsp, sizeof(whsp)); // size in bytes
	m_whitespaces = whsp;
	op = 0;
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
#else
	reg.Set(L"invert", inv);
	reg.Set(L"case", cs);
	reg.Set(L"numeric", ns);
	reg.Set(L"selection", sel);
	reg.Set(L"asempty", emp);
	reg.Set(L"whitespace", get_whitespaces().c_str());
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
