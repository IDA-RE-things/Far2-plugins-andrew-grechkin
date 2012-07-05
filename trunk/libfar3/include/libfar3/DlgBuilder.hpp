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

///=================================================================================================
struct DialogItemBinding_i {
	DialogItemBinding_i();

	virtual ~DialogItemBinding_i();

	virtual void SaveValue(FarDialogItem * Item, int RadioGroupIndex) = 0;

	int BeforeLabelId;
	int AfterLabelId;
};


///=================================================================================================
ssize_t TextWidth(const FarDialogItem * Item);

ssize_t ItemWidth(const FarDialogItem * Item);


///=================================================================================================
struct DialogBuilder_i {
	// Добавляет статический текст, расположенный на отдельной строке в диалоге.
	FarDialogItem * AddText(int LabelId) {
		return AddText_(LabelId);
	}

	// Добавляет статический текст, расположенный на отдельной строке в диалоге.
	FarDialogItem * AddText(PCWSTR Label) {
		return DoAddText(Label);
	}

	// Добавляет чекбокс.
	FarDialogItem * AddCheckbox(int LabelId, ssize_t * Value, ssize_t Mask = 0, bool ThreeState = false) {
		return DoAddCheckbox(LabelId, Value, Mask, ThreeState);
	}

	// Добавляет группу радиокнопок.
	void AddRadioButtons(ssize_t * Value, ssize_t OptionCount, const int MessageIds[], bool FocusOnSelected = false) {
		DoAddRadioButtons(Value, OptionCount, MessageIds, FocusOnSelected);
	}

	FarDialogItem * AddEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width = -1, PCWSTR HistoryId = nullptr, bool UseLastHistory = false) {
		return DoAddEditField(Value, MaxSize, Width, HistoryId, UseLastHistory);
	}

	// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
	FarDialogItem * AddIntEditField(ssize_t * Value, ssize_t Width) {
		return DoAddIntEditField(Value, Width);
	}

	FarDialogItem * AddFixEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask = nullptr) {
		return DoAddFixEditField(Value, MaxSize, Width, Mask);
	}

	FarDialogItem * AddPasswordField(PWSTR Value, ssize_t MaxSize, ssize_t Width) {
		return DoAddPasswordField(Value, MaxSize, Width);
	}

	// Добавляет указанную текстовую строку слева от элемента RelativeTo.
	FarDialogItem * AddTextBefore(PCWSTR Label, FarDialogItem * RelativeTo) {
		return DoAddTextBefore(Label, RelativeTo);
	}

	FarDialogItem * AddTextBefore(int LabelId, FarDialogItem * RelativeTo) {
		return DoAddTextBefore(get_lang_string(LabelId), RelativeTo);
	}

	// Добавляет указанную текстовую строку справа от элемента RelativeTo.
	FarDialogItem * AddTextAfter(PCWSTR Label, FarDialogItem * RelativeTo) {
		return DoAddTextAfter(Label, RelativeTo);
	}

	FarDialogItem * AddTextAfter(int LabelId, FarDialogItem * RelativeTo) {
		return DoAddTextAfter(get_lang_string(LabelId), RelativeTo);
	}

	// Добавляет кнопку справа от элемента RelativeTo.
	FarDialogItem * AddButtonAfter(PCWSTR Label, FarDialogItem * RelativeTo) {
		return DoAddButtonAfter(Label, RelativeTo);
	}

	FarDialogItem * AddButtonAfter(int LabelId, FarDialogItem * RelativeTo) {
		return DoAddButtonAfter(get_lang_string(LabelId), RelativeTo);
	}

	// Начинает располагать поля диалога в две колонки.
	void StartColumns() {
		DoStartColumns();
	}

	// Завершает колонку полей в диалоге и переходит к следующей колонке.
	void ColumnBreak() {
		DoColumnBreak();
	}

	// Завершает расположение полей диалога в две колонки.
	void EndColumns() {
		DoEndColumns();
	}

	// Начинает располагать поля диалога внутри single box
	void StartSingleBox(ssize_t Width, int LabelId = -1, bool LeftAlign = false) {
		DoStartSingleBox(Width, LabelId, LeftAlign);
	}

	// Завершает расположение полей диалога внутри single box
	void EndSingleBox() {
		DoEndSingleBox();
	}

	// Добавляет пустую строку.
	void AddEmptyLine() {
		DoAddEmptyLine();
	}

	// Добавляет сепаратор.
	void AddSeparator(int LabelId = -1) {
		DoAddSeparator(LabelId);
	}

	// Добавляет сепаратор, кнопки OK и Cancel.
	void AddOKCancel(int OKLabelId, int CancelLabelId, int ExtraLabelId = -1, bool Separator = true) {
		DoAddOKCancel(OKLabelId, CancelLabelId, ExtraLabelId, Separator);
	}

	int ShowDialogEx() {
		return DoShowDialogEx();
	}

	bool ShowDialog() {
		return ShowDialogEx() == 0;
	}

	int get_last_id() const {
		return get_last_id_();
	}

	virtual ~DialogBuilder_i();

private:
	virtual FarDialogItem * AddText_(int LabelId) = 0;

	virtual FarDialogItem * DoAddText(PCWSTR Label) = 0;

	virtual FarDialogItem * DoAddCheckbox(int LabelId, ssize_t * Value, ssize_t Mask, bool ThreeState) = 0;

	virtual void DoAddRadioButtons(ssize_t * Value, ssize_t OptionCount, const int LabelIds[], bool FocusOnSelected) = 0;

	virtual FarDialogItem * DoAddEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) = 0;

	virtual FarDialogItem * DoAddIntEditField(ssize_t * Value, ssize_t Width) = 0;

	virtual FarDialogItem * DoAddFixEditField(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) = 0;

	virtual FarDialogItem * DoAddPasswordField(PWSTR Value, ssize_t MaxSize, ssize_t Width) = 0;

	virtual FarDialogItem * DoAddTextBefore(PCWSTR Label, FarDialogItem * RelativeTo) = 0;

	virtual FarDialogItem * DoAddTextAfter(PCWSTR Label, FarDialogItem * RelativeTo) = 0;

	virtual FarDialogItem * DoAddButtonAfter(PCWSTR Label, FarDialogItem * RelativeTo) = 0;

	virtual void DoStartColumns() = 0;

	virtual void DoColumnBreak() = 0;

	virtual void DoEndColumns() = 0;

	virtual void DoStartSingleBox(ssize_t Width, int LabelId, bool LeftAlign) = 0;

	virtual void DoEndSingleBox() = 0;

	virtual void DoAddEmptyLine() = 0;

	virtual void DoAddSeparator(int LabelId) = 0;

	virtual void DoAddOKCancel(int OKLabelId, int CancelLabelId, int ExtraLabelId, bool Separator) = 0;

	virtual int DoShowDialog() = 0;

	virtual int DoShowDialogEx() = 0;

	virtual int get_last_id_() const = 0;

	virtual DialogItemBinding_i * DoCreateCheckBoxBinding(ssize_t * Value, ssize_t Mask) = 0;

	virtual DialogItemBinding_i * DoCreateRadioButtonBinding(ssize_t * Value) = 0;

	virtual PCWSTR get_lang_string(int LabelId) const = 0;

};


///=================================================================================================
DialogBuilder_i * get_dialog_builder(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

DialogBuilder_i * get_dialog_builder(const PluginStartupInfo & psi, const GUID & pluginId, const GUID & aId, int TitleLabelId, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);


///=================================================================================================
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

#endif
