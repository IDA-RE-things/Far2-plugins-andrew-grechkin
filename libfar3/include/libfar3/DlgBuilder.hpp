#ifndef __DLGBUILDER_HPP__
#define __DLGBUILDER_HPP__

/*
 Copyright © 2009 Far Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. The name of the authors may not be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <libfar3/plugin.hpp>

namespace Far {


	struct AddRadioButton_t {
		ssize_t id;
		FARDIALOGITEMFLAGS flags;
	};


	///=============================================================================================
	struct DialogBuilder_i {
		// Добавляет статический текст, расположенный на отдельной строке в диалоге.
		FarDialogItem * add_text(PCWSTR Label) {
			return add_text_(Label);
		}

		// Добавляет чекбокс.
		FarDialogItem * add_checkbox(PCWSTR Label, ssize_t * Value, ssize_t Mask = 0, bool ThreeState = false) {
			return add_checkbox_(Label, Value, Mask, ThreeState);
		}

		// Добавляет группу радиокнопок.
		void add_radiobuttons(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected = false) {
			add_radiobuttons_(Value, OptionCount, list, FocusOnSelected);
		}

		FarDialogItem * add_editfield(PWSTR Value, ssize_t MaxSize, ssize_t Width = -1, PCWSTR HistoryId = nullptr, bool UseLastHistory = false) {
			return add_editfield_(Value, MaxSize, Width, HistoryId, UseLastHistory);
		}

		// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
		FarDialogItem * add_inteditfield(ssize_t * Value, ssize_t Width) {
			return add_inteditfield_(Value, Width);
		}

		FarDialogItem * add_fixeditfield(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask = nullptr) {
			return add_fixeditfield_(Value, MaxSize, Width, Mask);
		}

		FarDialogItem * add_passwordfield(PWSTR Value, ssize_t MaxSize, ssize_t Width) {
			return add_passwordfield_(Value, MaxSize, Width);
		}

		// Добавляет указанную текстовую строку слева от элемента RelativeTo.
		FarDialogItem * add_text_before(PCWSTR Label, FarDialogItem * RelativeTo) {
			return add_item_before_(DI_TEXT, Label, RelativeTo);
		}

		// Добавляет указанную текстовую строку справа от элемента RelativeTo.
		FarDialogItem * add_text_after(PCWSTR Label, FarDialogItem * RelativeTo) {
			return add_item_after_(DI_TEXT, Label, RelativeTo);
		}

		// Добавляет кнопку справа от элемента RelativeTo.
		FarDialogItem * add_button_after(PCWSTR Label, FarDialogItem * RelativeTo) {
			return add_item_after_(DI_TEXT, Label, RelativeTo);
		}

		// Начинает располагать поля диалога в две колонки.
		void start_column() {
			start_column_();
		}

		// Завершает колонку полей в диалоге и переходит к следующей колонке.
		void break_column() {
			break_column_();
		}

		// Завершает расположение полей диалога в две колонки.
		void end_column() {
			end_column_();
		}

		// Начинает располагать поля диалога внутри single box
		void start_singlebox(ssize_t Width, PCWSTR Label = Base::EMPTY_STR, bool LeftAlign = false) {
			start_singlebox_(Width, Label, LeftAlign);
		}

		// Завершает расположение полей диалога внутри single box
		void end_singlebox() {
			end_singlebox_();
		}

		// Добавляет пустую строку.
		void add_empty_line() {
			add_empty_line_();
		}

		// Добавляет сепаратор.
		void add_separator(PCWSTR Label = Base::EMPTY_STR) {
			add_separator_(Label);
		}

		// Добавляет сепаратор, кнопки OK и Cancel.
		void add_OKCancel(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel = Base::EMPTY_STR, bool Separator = true) {
			add_OKCancel_(OKLabel, CancelLabel, ExtraLabel, Separator);
		}

		int show_ex() {
			return show_dialog_ex_();
		}

		bool show() {
			return show_ex() == 0;
		}

		virtual ~DialogBuilder_i();

	private:
		virtual FarDialogItem * add_text_(PCWSTR Label) = 0;

		virtual FarDialogItem * add_checkbox_(PCWSTR Label, ssize_t * Value, ssize_t Mask, bool ThreeState) = 0;

		virtual void add_radiobuttons_(ssize_t * Value, ssize_t OptionCount, const AddRadioButton_t list[], bool FocusOnSelected) = 0;

		virtual FarDialogItem * add_editfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) = 0;

		virtual FarDialogItem * add_fixeditfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) = 0;

		virtual FarDialogItem * add_inteditfield_(ssize_t * Value, ssize_t Width) = 0;

		virtual FarDialogItem * add_passwordfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width) = 0;

		virtual FarDialogItem * add_item_before_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) = 0;

		virtual FarDialogItem * add_item_after_(FARDIALOGITEMTYPES Type, PCWSTR Label, FarDialogItem * RelativeTo) = 0;

		virtual void start_column_() = 0;

		virtual void break_column_() = 0;

		virtual void end_column_() = 0;

		virtual void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) = 0;

		virtual void end_singlebox_() = 0;

		virtual void add_empty_line_() = 0;

		virtual void add_separator_(PCWSTR Label) = 0;

		virtual void add_OKCancel_(PCWSTR OKLabel, PCWSTR CancelLabel, PCWSTR ExtraLabel, bool Separator) = 0;

		virtual int show_dialog_() = 0;

		virtual int show_dialog_ex_() = 0;
	};


	///=============================================================================================
	DialogBuilder_i * get_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);


	///=============================================================================================
	struct DialogBuilder {
		~DialogBuilder() {
			delete m_ptr;
		}

		DialogBuilder(DialogBuilder_i * ptr):
			m_ptr(ptr) {
		}

		DialogBuilder(DialogBuilder && rhs):
			m_ptr(rhs.m_ptr) {
			rhs.m_ptr = nullptr;
		}

		DialogBuilder & operator = (DialogBuilder && rhs) {
			if (this != &rhs) {
				DialogBuilder_i * tmp = m_ptr;
				m_ptr = rhs.m_ptr;
				rhs.m_ptr = nullptr;
				delete tmp;
			}
			return *this;
		}

		DialogBuilder_i * operator -> () const {
			return m_ptr;
		}

	private:
		DialogBuilder(const DialogBuilder &);

		DialogBuilder & operator = (const DialogBuilder &);

		DialogBuilder_i * m_ptr;
	};

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
