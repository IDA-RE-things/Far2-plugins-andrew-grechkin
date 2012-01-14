#include "farplugin.hpp"

ustring make_path(const ustring & path, const ustring & name) {
	return path + PATH_SEPARATOR + name;
}

Options::Options() {
}

void Options::load() {
#ifndef FAR2
	m_settings.reset(new Far::Settings_t(FarPlugin::get_guid()));
#else
	m_settings.reset(new Register);
	m_settings->open_key(KEY_READ | KEY_WRITE, make_path(Far::get_plugin_path(), plugin->get_name()).c_str());
#endif
	op = m_settings->get(L"operation", 1);
	opm = m_settings->get(L"is_text_mask", 0);
}

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
	m_settings->set(L"operation", op);
	m_settings->set(L"is_text_mask", opm);
}

size_t Options::get_current_line() const {
	return m_ei.CurLine;
}

size_t Options::get_current_column() const {
	return m_ei.CurPos;
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
