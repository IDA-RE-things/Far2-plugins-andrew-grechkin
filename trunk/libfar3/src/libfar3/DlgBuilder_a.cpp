#include "DlgBuilder_pvt.hpp"

namespace Far {

	DialogItemBinding::~DialogItemBinding()
	{
	}

	DialogItemBinding::DialogItemBinding(HANDLE & dlg, FarDialogItem * Item, ssize_t index):
		m_dlg(dlg),
		m_index(index),
		m_item(Item) {
	}

	HANDLE DialogItemBinding::get_dlg() const {
		return m_dlg;
	}

	ssize_t DialogItemBinding::get_index() const {
		return m_index;
	}

	FarDialogItem * DialogItemBinding::get_item() const {
		return m_item;
	}

}
