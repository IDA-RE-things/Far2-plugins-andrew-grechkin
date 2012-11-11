#ifndef __FAR_DIALOG_BUIDER_EX_HPP__
#define __FAR_DIALOG_BUIDER_EX_HPP__

#include <libfar3/dialog_builder.hpp>

namespace Far {

	///=============================================================================================
	struct ExtendedDialogBuilder_i: public SimpleDialogBuilder_i {
		FarDialogItem * add_editfield(PWSTR Value, ssize_t MaxSize, ssize_t Width = -1, PCWSTR HistoryId = nullptr, bool UseLastHistory = false)
		{
			return add_editfield_(Value, MaxSize, Width, HistoryId, UseLastHistory);
		}

		// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
		FarDialogItem * add_inteditfield(ssize_t * Value, ssize_t Width)
		{
			return add_inteditfield_(Value, Width);
		}

		FarDialogItem * add_fixeditfield(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask = nullptr)
		{
			return add_fixeditfield_(Value, MaxSize, Width, Mask);
		}

		FarDialogItem * add_passwordfield(PWSTR Value, ssize_t MaxSize, ssize_t Width)
		{
			return add_passwordfield_(Value, MaxSize, Width);
		}

		//		// Добавляет кнопку справа от элемента RelativeTo.
		//		FarDialogItem * add_button_after(PCWSTR Label, FarDialogItem * RelativeTo)
		//		{
		//			return add_item_after_(DI_TEXT, Label, RelativeTo);
		//		}
		//
		// Начинает располагать поля диалога в две колонки.
		void start_column()
		{
			start_column_();
		}

		// Завершает колонку полей в диалоге и переходит к следующей колонке.
		void break_column()
		{
			break_column_();
		}

		// Завершает расположение полей диалога в две колонки.
		void end_column()
		{
			end_column_();
		}

		// Начинает располагать поля диалога внутри single box
		void start_singlebox(ssize_t Width, PCWSTR Label = Base::EMPTY_STR, bool LeftAlign = false)
		{
			start_singlebox_(Width, Label, LeftAlign);
		}

		// Завершает расположение полей диалога внутри single box
		void end_singlebox()
		{
			end_singlebox_();
		}

		virtual ~ExtendedDialogBuilder_i();

	private:
		virtual FarDialogItem * add_editfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR HistoryId, bool UseLastHistory) = 0;

		virtual FarDialogItem * add_fixeditfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width, PCWSTR Mask) = 0;

		virtual FarDialogItem * add_inteditfield_(ssize_t * Value, ssize_t Width) = 0;

		virtual FarDialogItem * add_passwordfield_(PWSTR Value, ssize_t MaxSize, ssize_t Width) = 0;

		virtual void start_column_() = 0;

		virtual void break_column_() = 0;

		virtual void end_column_() = 0;

		virtual void start_singlebox_(ssize_t Width, PCWSTR Label, bool LeftAlign) = 0;

		virtual void end_singlebox_() = 0;

	};

	///=============================================================================================
	ExtendedDialogBuilder_i * get_extended_dialog_builder(const GUID & aId, PCWSTR TitleLabel, PCWSTR aHelpTopic = nullptr, FARWINDOWPROC aDlgProc = nullptr, void * aUserParam = nullptr);

}

#endif
