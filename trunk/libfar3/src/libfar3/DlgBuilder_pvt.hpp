#include <libfar3/helper.hpp>
#include <libfar3/DlgBuilder.hpp>

namespace Far {

	struct DialogItemBinding {
		HANDLE get_dlg() const;

		ssize_t get_index() const;

		FarDialogItem * get_item() const;

		virtual ssize_t get_width() const = 0;

		virtual void save() const = 0;

		virtual ~DialogItemBinding();

	protected:
		DialogItemBinding(HANDLE & dlg, FarDialogItem * Item, ssize_t index);

	private:
		HANDLE & m_dlg;
		ssize_t m_index;
		FarDialogItem * m_item;
	};


	struct PluginCheckBoxBinding: public DialogItemBinding {
		PluginCheckBoxBinding(HANDLE & aHandle, FarDialogItem * Item, ssize_t aId, ssize_t * aValue, ssize_t aMask);

		virtual void save() const;

		virtual ssize_t get_width() const;

	private:
		ssize_t * Value;
		ssize_t  Mask;
	};


	struct PluginRadioButtonBinding: public DialogItemBinding {
		PluginRadioButtonBinding(HANDLE & aHandle, FarDialogItem * Item, ssize_t aId, ssize_t * aValue, ssize_t RadioGroupIndex);

		virtual void save() const;

		virtual ssize_t get_width() const;

	private:
		ssize_t * Value;
		ssize_t m_rg_index;
	};


	struct PluginEditFieldBinding: public DialogItemBinding {
		PluginEditFieldBinding(HANDLE & aHandle, FarDialogItem * Item, ssize_t aId, PWSTR aValue, int aMaxSize);

		virtual void save() const;

		virtual ssize_t get_width() const;

	private:
		PWSTR Value;
		int MaxSize;
	};


	struct PluginIntEditFieldBinding: public DialogItemBinding {
		PluginIntEditFieldBinding(HANDLE & aHandle, FarDialogItem * Item, ssize_t aId, ssize_t * aValue, ssize_t Width);

		virtual void save() const;

		virtual ssize_t get_width() const;

		PWSTR GetBuffer();

		PCWSTR GetMask() const;

	private:
		ssize_t * Value;
		wchar_t Buffer[32];
		wchar_t Mask[32];
	};

}
