#include "farplugin.hpp"

Options::Options() {
}

#ifndef FAR2
void Options::load() {
	m_settings.reset(new Far::Settings_t(*Far::guid()));
	op = m_settings->get(FSSF_ROOT, L"operation", 1);
	opm = m_settings->get(FSSF_ROOT, L"is_text_mask", 0);
}
#else
void Options::load(const ustring & path) {
	reg.open_key(KEY_READ | KEY_WRITE, path.c_str());
	op = reg.get(L"operation", 1);
	opm = reg.get(L"is_text_mask", 0);
}
#endif

void Options::get_parameters(const Far::Dialog & dlg) {
	for (int i = indDelAll; i <= indDelWithoutText; ++i) {
		if (dlg.Check(i)) {
			op = i;
			break;
		}
	}
	opm = dlg.Check(indIsMask);
}

void Options::save() const {
#ifndef FAR2
	m_settings->set(FSSF_ROOT, L"operation", op);
	m_settings->set(FSSF_ROOT, L"is_text_mask", opm);
#else
	reg.set(L"operation", op);
	reg.set(L"is_text_mask", opm);
#endif
}

size_t Options::get_first_line() const {
	return m_first_line;
}

size_t Options::get_current_line() const {
	return m_ei.CurLine;
}

size_t Options::get_current_column() const {
	return m_ei.CurPos;
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
