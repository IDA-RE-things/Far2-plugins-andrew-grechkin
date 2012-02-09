#include "farplugin.hpp"
#include "lang.hpp"
#include "guid.hpp"

windef::shared_ptr<FarPlugin> plugin;

bool FarPlugin::Execute() const {
	if (!options.get_total_lines())
		return false;

	Far::Editor::start_undo();
    size_t current = options.get_current_line();
    size_t total = options.get_total_lines();
	EditorGetString egs;
	for (size_t i = options.get_first_line(); i < total; ++i) {
		if (!Far::Editor::get_string(i, egs))
			break;

		if (i == (total - 1) && Empty(egs.StringText))
			break;

		if (options.get_block_type() != BTYPE_NONE && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;

		switch (options.op) {
			case 1:
				if (Empty(egs.StringText))
					delete_string(i, total, current);
				break;
			case 2:
				if (i > options.get_first_line()) {
					static EditorGetString pred;
					Far::Editor::get_string(i - 1, pred);
					if (Empty(pred.StringText) && Empty(egs.StringText)) {
						delete_string(i, total, current);
					}
				}
				break;
			case 3:
				if (options.opm) {
					if (Far::fsf().ProcessName((PWSTR)options.text.c_str(), (PWSTR)egs.StringText, 0, PN_CMPNAMELIST | PN_SKIPPATH)) {
						delete_string(i, total, current);
					}
				} else if (Find(egs.StringText, options.text.c_str())) {
					delete_string(i, total, current);
				}
				break;
			case 4:
				if (options.opm) {
					if (!Far::fsf().ProcessName((PWSTR)options.text.c_str(), (PWSTR)egs.StringText, 0, PN_CMPNAMELIST | PN_SKIPPATH)) {
						delete_string(i, total, current);
					}
				} else if (!Find(egs.StringText, options.text.c_str())) {
					delete_string(i, total, current);
				}
				break;
		}
	}
	Far::Editor::set_position(current, options.get_current_column());
	Far::Editor::stop_undo();

	Far::Editor::redraw();

	return true;
}

#ifndef FAR2
GUID FarPlugin::get_guid() {
	return PluginGuid;
}
#endif

PCWSTR FarPlugin::get_prefix() const {
	static PCWSTR ret = L"";
	return ret;
}

PCWSTR FarPlugin::get_name() {
	return L"delstr";
}

PCWSTR FarPlugin::get_description() {
	return L"Delete strings in editor. FAR2, FAR3 plugin";
}

PCWSTR FarPlugin::get_author() {
	return L"© 2012 Andrew Grechkin";
}

FarPlugin::FarPlugin(const PluginStartupInfo * psi) {
#ifndef FAR2
	Far::helper_t::inst().init(FarPlugin::get_guid(), psi);
#else
	Far::helper_t::inst().init(psi);
#endif
	options.load();
}

void FarPlugin::get_info(PluginInfo * pi) const {
	pi->StructSize = sizeof(*pi);
	pi->Flags = PF_DISABLEPANELS | PF_EDITOR;
	static PCWSTR PluginMenuStrings[] = {Far::get_msg(Far::MenuTitle)};
#ifndef FAR2
	pi->PluginMenu.Guids = &MenuGuid;
	pi->PluginMenu.Strings = PluginMenuStrings;
	pi->PluginMenu.Count = lengthof(PluginMenuStrings);
#else
	pi->PluginMenuStrings = PluginMenuStrings;
	pi->PluginMenuStringsNumber = lengthof(PluginMenuStrings);
#endif
}

#ifndef FAR2
HANDLE FarPlugin::open(const OpenInfo * /*Info*/)
#else
HANDLE FarPlugin::open(int /*OpenFrom*/, INT_PTR /*Item*/)
#endif
{
	Far::InitDialogItemF Items[] = {
		{DI_DOUBLEBOX, 3, 1, WIDTH - 4, HEIGHT - 2, 0, (PCWSTR)Far::DlgTitle},
		{DI_RADIOBUTTON, 5, 2, 54, 0, 0, (PCWSTR)rbDelAll},
		{DI_RADIOBUTTON, 5, 3, 54, 0, 0, (PCWSTR)rbDelRepeated},
		{DI_RADIOBUTTON, 5, 4, 54, 0, 0, (PCWSTR)rbDelWithText},
		{DI_RADIOBUTTON, 5, 5, 54, 0, 0, (PCWSTR)rbDelWithoutText},
		{DI_EDIT, 9, 6, 46, 0, 0, options.text.c_str()},
		{DI_CHECKBOX, 50, 6, 0, 0, 0, (PCWSTR)rbMask},
		{DI_TEXT, 0, HEIGHT - 4, 0, 0, DIF_SEPARATOR, EMPTY_STR},
		{DI_BUTTON, 0, HEIGHT - 3, 0, 0, DIF_CENTERGROUP, (PCWSTR)Far::txtBtnOk},
		{DI_BUTTON, 0, HEIGHT - 3, 0, 0, DIF_CENTERGROUP, (PCWSTR)Far::txtBtnCancel},
	};
	size_t size = lengthof(Items);

	options.load_editor_info();
	FarDialogItem FarItems[size];
	InitDialogItemsF(Items, FarItems, size);
	FarItems[options.op].Selected = 1;
	FarItems[indIsMask].Selected = options.opm;
#ifndef FAR2
	FarItems[size - 2].Flags |= DIF_DEFAULTBUTTON;
#else
	FarItems[size - 2].DefaultButton = 1;
#endif
	FarItems[indText].Flags |= DIF_HISTORY;
	FarItems[indText].History = L"delstr.text";

	Far::Dialog hDlg;
#ifndef FAR2
	if (hDlg.Init(DialogGuid, -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size))
#else
	if (hDlg.Init(Far::get_module_number(), -1, -1, WIDTH, HEIGHT, nullptr, FarItems, size))
#endif
	{
		int ret = hDlg.Run();
		if (ret > 0 && Items[ret].Data == (PCWSTR)Far::txtBtnOk) {
			options.get_parameters(hDlg);
			if ((options.op == indDelWithText) || (options.op == indDelWithoutText))
				options.text = hDlg.Str(indText);
			options.save();
			Execute();
		}
	}
	return INVALID_HANDLE_VALUE;
}

void FarPlugin::delete_string(size_t & index, size_t & total, size_t & current) const {
	if (index <= current)
		current--;
	Far::Editor::del_string(index--);
	total--;
}
