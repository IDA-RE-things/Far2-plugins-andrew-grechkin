#include <libfar3/helper.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libfar3/dialog_builder_ex.hpp>

#include <vector>

namespace Far {

	ssize_t ItemWidth(const FarDialogItem * Item);

	///=============================================================================================
	struct SimpleDialogBuilder_impl: public SimpleDialogBuilder_i {
		SimpleDialogBuilder_impl(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

		~SimpleDialogBuilder_impl() override;

		FarDialogItem_t * add_text_(PCWSTR Label) override;

		FarDialogItem_t * add_item_before_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) override;

		FarDialogItem_t * add_item_after_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) override;

		FarDialogItem_t * add_checkbox_(PCWSTR Label, ssize_t * Value, ssize_t Mask, bool ThreeState) override;

		void add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected) override;

		void add_empty_line_() override;

		void add_separator_(PCWSTR Label) override;

		void add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel, bool Separator) override;

		int show_() override;

	protected:
		void add_border(PCWSTR Text);

		FarDialogItem_t * add_dialog_item(FARDIALOGITEMTYPES Type, PCWSTR Text, FARDIALOGITEMFLAGS flags = DIF_NONE);

		ssize_t MaxTextWidth();

		void save();

		void set_next_y(FarDialogItem_t * Item);

		int show_dialog_();

		void UpdateBorderSize();

	protected:
		GUID Id;
		FARWINDOWPROC DlgProc;
		PCWSTR HelpTopic;
		void * UserParam;

		HANDLE DialogHandle;

		std::vector<FarDialogItem_t> DialogItems;
		size_t DialogItemsAllocated;

		ssize_t Indent;
		ssize_t NextY;

		ssize_t OKButtonId;
	};

	///=============================================================================================
//	struct DialogBuilder_inst: public ExtendedDialogBuilder_i, private SimpleDialogBuilder_impl {
//		DialogBuilder_inst(const GUID & aId, PCWSTR Label, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);
//
//		using SimpleDialogBuilder_impl::add_text_;
//
//		~DialogBuilder_inst() override;
//
//		FarDialogItem * add_editfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) override;
//
//		FarDialogItem * add_fixeditfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) override;
//
//		FarDialogItem * add_inteditfield_(ssize_t * Value, ssize_t Width) override;
//
//		FarDialogItem * add_passwordfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width) override;
//
//		void start_column_() override;
//
//		void break_column_() override;
//
//		void end_column_() override;
//
//		void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) override;
//
//		void end_singlebox_() override;
//
//	private:
//		ssize_t get_last_id_() const;
//
//		DialogItemBinding_i * FindBinding(FarDialogItem * Item);
//
//		int GetItemId(FarDialogItem * Item);
//
//	private:
//		int SingleBoxIndex;
//
//		int ColumnStartIndex;
//		int ColumnBreakIndex;
//		int ColumnStartY;
//	};

	///=============================================================================================
	struct PluginCheckBoxBinding: public DialogItemBinding_i {
		PluginCheckBoxBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t aMask);

		void save_() const override;

		virtual ssize_t get_width() const;

	private:
		ssize_t * Value;
		ssize_t Mask;
	};

	struct PluginRadioButtonBinding: public DialogItemBinding_i {
		PluginRadioButtonBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t RadioGroupIndex);

		void save_() const override;

		virtual ssize_t get_width() const;

	private:
		ssize_t * Value;
		ssize_t m_rg_index;
	};

	struct PluginEditFieldBinding: public DialogItemBinding_i {
		PluginEditFieldBinding(HANDLE & aHandle, ssize_t aId, PWSTR aValue, ssize_t aMaxSize);

		void save_() const override;

		virtual ssize_t get_width() const;

	private:
		PWSTR Value;
		ssize_t MaxSize;
	};

	struct PluginIntEditFieldBinding: public DialogItemBinding_i {
		PluginIntEditFieldBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t Width);

		void save_() const override;

		virtual ssize_t get_width() const;

		PWSTR GetBuffer();

		PCWSTR GetMask() const;

	private:
		ssize_t * Value;
		ssize_t m_width;
		wchar_t Buffer[32];
		wchar_t Mask[32];
	};

}
