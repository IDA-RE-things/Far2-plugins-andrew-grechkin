#ifndef __FAR_DIALOG_BUIDER_HPP__
#define __FAR_DIALOG_BUIDER_HPP__

#include <libfar3/plugin.hpp>
#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>

namespace Far {

	struct DialogItemBinding_i {
		HANDLE get_dlg() const
		{
			return m_dlg;
		}

		ssize_t get_index() const
		{
			return m_index;
		}

		void save() const;

		virtual ~DialogItemBinding_i();

	protected:
		DialogItemBinding_i(HANDLE & dlg, ssize_t index):
			m_dlg(dlg),
			m_index(index)
		{
		}

	private:
		virtual void save_() const = 0;

	HANDLE & m_dlg;
		ssize_t m_index;
	};

	struct FarDialogItem_t: public FarDialogItem {
//		FarDialogItem_t()
//		{
//			::memset(this, 0, sizeof(*this));
//		}

		FarDialogItem_t(FARDIALOGITEMTYPES Type_, PCWSTR Text_, FARDIALOGITEMFLAGS flags_ = DIF_NONE)
		{
			::memset(this, 0, sizeof(*this));
			Type = Type_;
			Data = Text_;
			Flags = flags_;
		}

		void set_binding(DialogItemBinding_i * binding)
		{
			UserData = (intptr_t)binding;
		}

//		DialogItemBinding_i * get_binding() const
//		{
//			return (DialogItemBinding_i*)UserData;
//		}

		ssize_t get_width() const;

		void save() const
		{
			((DialogItemBinding_i*)UserData)->save();
		}

		void destroy()
		{
			delete (DialogItemBinding_i*)UserData;
			UserData = 0;
		}

	};

	struct AddRadioButton_t {
		ssize_t id;
		FARDIALOGITEMFLAGS flags;
	};

	///=============================================================================================
	struct SimpleDialogBuilder_i {
		// Добавляет статический текст, расположенный на отдельной строке в диалоге.
		FarDialogItem_t * add_text(PCWSTR Label)
		{
			return add_text_(Label);
		}

		// Добавляет указанную текстовую строку слева от элемента RelativeTo.
		FarDialogItem_t * add_text_before(PCWSTR Label, FarDialogItem * RelativeTo)
		{
			return add_item_before_(DI_TEXT, Label, RelativeTo);
		}

		// Добавляет указанную текстовую строку справа от элемента RelativeTo.
		FarDialogItem_t * add_text_after(PCWSTR Label, FarDialogItem * RelativeTo)
		{
			return add_item_after_(DI_TEXT, Label, RelativeTo);
		}

		// Добавляет чекбокс.
		FarDialogItem_t * add_checkbox(PCWSTR Label, ssize_t * Value, ssize_t Mask = 0, bool ThreeState = false)
		{
			return add_checkbox_(Label, Value, Mask, ThreeState);
		}

		// Добавляет группу радиокнопок.
		void add_radiobuttons(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected = false)
		{
			add_radiobuttons_(Value, OptionCount, list, FocusOnSelected);
		}

		// Добавляет пустую строку.
		void add_empty_line()
		{
			add_empty_line_();
		}

		// Добавляет сепаратор.
		void add_separator(PCWSTR Label = Base::EMPTY_STR)
		{
			add_separator_(Label);
		}

		// Добавляет сепаратор, кнопки OK и Cancel.
		void add_OKCancel(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel = Base::EMPTY_STR, bool Separator = true)
		{
			add_OKCancel_(OKLabel, CancelLabel, ExtraLabel, Separator);
		}

		int show_ex()
		{
			return show_();
		}

		bool show()
		{
			return show_() == 0;
		}

		virtual ~SimpleDialogBuilder_i();

	private:
		virtual FarDialogItem_t * add_text_(PCWSTR Label) = 0;

		virtual FarDialogItem_t * add_item_before_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) = 0;

		virtual FarDialogItem_t * add_item_after_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) = 0;

		virtual FarDialogItem_t * add_checkbox_(PCWSTR Label, ssize_t * Value, ssize_t Mask, bool ThreeState) = 0;

		virtual void add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected) = 0;

		virtual void add_empty_line_() = 0;

		virtual void add_separator_(PCWSTR Label) = 0;

		virtual void add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel, bool Separator) = 0;

		virtual int show_() = 0;

	};

	///=============================================================================================
	SimpleDialogBuilder_i * get_simple_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

	///=============================================================================================
//	struct DialogBuilder: private Base::Uncopyable {
//		~DialogBuilder()
//		{
//			delete m_ptr;
//		}
//
//		DialogBuilder(DialogBuilder_i * ptr) :
//			m_ptr(ptr)
//		{
//		}
//
//		DialogBuilder(DialogBuilder && rhs):
//		m_ptr(nullptr)
//		{
//			swap(rhs);
//		}
//
//		void swap(DialogBuilder & rhs) {
//			using std::swap;
//			swap(m_ptr, rhs.m_ptr);
//		}
//
//		DialogBuilder & operator = (DialogBuilder && rhs) {
//			if (this != &rhs) {
//				DialogBuilder(std::move(rhs)).swap(*this);
//			}
//			return *this;
//		}
//
//		DialogBuilder_i * operator -> () const {
//			return m_ptr;
//		}
//
//	private:
//		DialogBuilder_i * m_ptr;
//	};

}

//	///====================================================================================== Dialog
//	struct Dialog {
//		~Dialog() {
//			Free();
//		}
//
//		Dialog() :
//			m_hndl(INVALID_HANDLE_VALUE) {
//		}
//
//		bool Init(const GUID & dguid, int X1, int Y1, int X2, int Y2, PCWSTR HelpTopic, FarDialogItem* Item, int ItemsNumber, DWORD Reserved = 0, DWORD Flags = 0, FARWINDOWPROC DlgProc = nullptr,
//		          PVOID Param = nullptr) {
//			Free();
//			m_hndl = psi().DialogInit(get_plugin_guid(), &dguid, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
//			return (m_hndl && m_hndl != INVALID_HANDLE_VALUE);
//		}
//
//		int Run() {
//			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE)
//				return psi().DialogRun(m_hndl);
//			return -1;
//		}
//
//		HANDLE Handle() const {
//			return m_hndl;
//		}
//
//		operator HANDLE() const {
//			return m_hndl;
//		}
//
//		int Check(int index) const {
//			return (int)psi().SendDlgMessage(m_hndl, DM_GETCHECK, index, 0);
//		}
//
//		bool IsChanged(int index) const {
//			return !(bool)psi().SendDlgMessage(m_hndl, DM_EDITUNCHANGEDFLAG, index, nullptr);
//		}
//
//		PCWSTR Str(int index) const {
//			return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, index, nullptr);
//		}
//
//		DWORD Flags(int index) {
//			FarDialogItem DialogItem;
//			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ? DialogItem.Flags : 0;
//		}
//
//		DWORD Type(int index) {
//			FarDialogItem DialogItem;
//			return psi().SendDlgMessage(m_hndl, DM_GETDLGITEMSHORT, index, &DialogItem) ? DialogItem.Type : 0;
//		}
//
//		ssize_t get_list_position(int index) const {
//			return psi().SendDlgMessage(m_hndl, DM_LISTGETCURPOS, index, 0);
//		}
//
//	private:
//		void Free() {
//			if (m_hndl && m_hndl != INVALID_HANDLE_VALUE) {
//				psi().DialogFree(m_hndl);
//				m_hndl = INVALID_HANDLE_VALUE;
//			}
//		}
//
//		HANDLE m_hndl;
//	};
//
//	inline PCWSTR get_data_ptr(HANDLE m_hndl, size_t in) {
//		return (PCWSTR)psi().SendDlgMessage(m_hndl, DM_GETCONSTTEXTPTR, in, 0);
//	}
//
//	inline bool get_checkbox(HANDLE m_hndl, size_t in) {
//		return (bool)psi().SendDlgMessage(m_hndl, DM_GETCHECK, in, 0);
//	}

#endif
