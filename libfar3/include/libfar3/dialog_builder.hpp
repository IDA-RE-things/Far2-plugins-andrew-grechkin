/**
 © 2012 Andrew Grechkin
 Source code: <http://code.google.com/p/andrew-grechkin>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef _LIBFAR_DIALOG_BUIDER_HPP_
#define _LIBFAR_DIALOG_BUIDER_HPP_

#include <libfar3/plugin.hpp>
#include <libfar3/helper.hpp>

#include <libbase/std.hpp>
#include <libbase/shared_ptr.hpp>

namespace Far {

	struct DialogItemBinding_i {
		virtual ~DialogItemBinding_i();

		HANDLE get_dlg() const
		{
			return *m_dlg;
		}

		ssize_t get_index() const
		{
			return m_index;
		}

		void set_dlg(HANDLE * hndl);

		void set_index(ssize_t index);

		void save() const;

		ssize_t get_height() const;

		ssize_t get_width() const;

	protected:
		DialogItemBinding_i() :
			m_dlg(),
			m_index()
		{
		}

		DialogItemBinding_i(HANDLE * dlg, ssize_t index) :
			m_dlg(dlg),
			m_index(index)
		{
		}

	private:
		virtual void save_() const = 0;

		virtual ssize_t get_height_() const;

		virtual ssize_t get_width_() const = 0;

		HANDLE * m_dlg;
		ssize_t m_index;
	};

	struct FarDialogItem_t: public FarDialogItem {
		~FarDialogItem_t();

		FarDialogItem_t(FARDIALOGITEMTYPES Type_, PCWSTR Text_, FARDIALOGITEMFLAGS flags_ = DIF_NONE);

		FarDialogItem_t(DialogItemBinding_i * binding, FARDIALOGITEMTYPES Type_, PCWSTR Text_, FARDIALOGITEMFLAGS flags_ = DIF_NONE);

		FarDialogItem_t(FarDialogItem_t && right);

		FarDialogItem_t & operator = (FarDialogItem_t && right);

		ssize_t get_height() const;

		ssize_t get_width() const;

		void set_dlg(HANDLE * dlg);

		void set_index(ssize_t ind);

		void save() const;

	private:
		FarDialogItem_t(FarDialogItem_t const& right) = delete;

		FarDialogItem_t & operator = (FarDialogItem_t const& right) = delete;
	};

	FarDialogItem_t * create_label(PCWSTR text, FARDIALOGITEMFLAGS flags = DIF_NONE);

	inline FarDialogItem_t * create_label(ssize_t msg_id, FARDIALOGITEMFLAGS flags = DIF_NONE)
	{
		return create_label(get_msg(msg_id), flags);
	}

	FarDialogItem_t * create_separator(PCWSTR text = nullptr, FARDIALOGITEMFLAGS flags = DIF_NONE);

	inline FarDialogItem_t * create_separator(ssize_t msg_id, FARDIALOGITEMFLAGS flags = DIF_NONE)
	{
		return create_separator(get_msg(msg_id), flags);
	}

	FarDialogItem_t * create_checkbox(ssize_t * value, PCWSTR text, FARDIALOGITEMFLAGS flags = DIF_NONE);

	inline FarDialogItem_t * create_checkbox(ssize_t * value, ssize_t msg_id, FARDIALOGITEMFLAGS flags = DIF_NONE)
	{
		return create_checkbox(value, get_msg(msg_id), flags);
	}

	FarDialogItem_t * create_combobox(ssize_t * value, FarList * items, FARDIALOGITEMFLAGS flags = DIF_NONE);

	FarDialogItem_t * create_edit(PWSTR value, ssize_t max_size, ssize_t width = -1, PCWSTR history_id = nullptr, bool use_last_history = false, FARDIALOGITEMFLAGS flags = DIF_NONE);

	FarDialogItem_t * create_password(PWSTR value, ssize_t max_size, ssize_t width = -1, FARDIALOGITEMFLAGS flags = DIF_NONE);

	struct AddRadioButton_t {
		ssize_t id;
		FARDIALOGITEMFLAGS flags;
	};

//	FarDialogItem_t * create_radio(ssize_t * Value, );

	///=============================================================================================
	struct DialogBuilder_i {
		virtual ~DialogBuilder_i();

		FarDialogItem_t * add_item(FarDialogItem_t * item)
		{
			return add_item_(item);
		}

		FarDialogItem_t * add_item_before(FarDialogItem_t * item)
		{
			return add_item_before_(item);
		}

		FarDialogItem_t * add_item_after(FarDialogItem_t * item)
		{
			return add_item_after_(item);
		}

		void add_empty_line()
		{
			add_empty_line_();
		}

		void add_OKCancel(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel = nullptr)
		{
			add_OKCancel_(OKLabel, CancelLabel, ExtraLabel);
		}

//		// Добавляет группу радиокнопок.
//		void add_radiobuttons(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected = false)
//		{
//			add_radiobuttons_(Value, OptionCount, list, FocusOnSelected);
//		}

		void start_column()
		{
			start_column_();
		}

		void break_column()
		{
			break_column_();
		}

		void end_column()
		{
			end_column_();
		}

		void start_singlebox(ssize_t Width, PCWSTR Label = Base::EMPTY_STR, bool LeftAlign = false)
		{
			start_singlebox_(Width, Label, LeftAlign);
		}

		void end_singlebox()
		{
			end_singlebox_();
		}

		int show_ex()
		{
			return show_();
		}

		bool show()
		{
			return show_() == 0;
		}

	private:
		virtual FarDialogItem_t * add_item_(FarDialogItem_t * item) = 0;

		virtual FarDialogItem_t * add_item_before_(FarDialogItem_t * item) = 0;

		virtual FarDialogItem_t * add_item_after_(FarDialogItem_t * item) = 0;

		virtual void add_empty_line_() = 0;

		virtual void add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel) = 0;

//		virtual void add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected) = 0;

		virtual void start_column_() = 0;

		virtual void break_column_() = 0;

		virtual void end_column_() = 0;

		virtual void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) = 0;

		virtual void end_singlebox_() = 0;

		virtual int show_() = 0;
	};

	///=============================================================================================
	Base::shared_ptr<DialogBuilder_i> create_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

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
