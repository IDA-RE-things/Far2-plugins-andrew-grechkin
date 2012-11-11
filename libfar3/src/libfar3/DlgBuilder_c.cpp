#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginRadioButtonBinding::PluginRadioButtonBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t RadioGroupIndex) :
		DialogItemBinding_i(aHandle, aId),
		Value(aValue),
		m_rg_index(RadioGroupIndex)
	{
	}

	void PluginRadioButtonBinding::save_() const
	{
		if (psi().SendDlgMessage(get_dlg(), DM_GETCHECK, get_index(), nullptr)) {
			*Value = m_rg_index;
		}
	}

	ssize_t PluginRadioButtonBinding::get_width() const
	{
//		return ::lstrlenW(get_item()->Data) + 4;
		return 0;
	}

}
