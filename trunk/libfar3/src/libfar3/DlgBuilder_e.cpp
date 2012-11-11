#include "DlgBuilder_pvt.hpp"

namespace Far {

	PluginIntEditFieldBinding::PluginIntEditFieldBinding(HANDLE & aHandle, ssize_t aId, ssize_t * aValue, ssize_t Width) :
		DialogItemBinding_i(aHandle, aId),
		Value(aValue),
		m_width(Width < 31 ? Width : 31)
	{
		fsf().sprintf(Buffer, L"%u", *aValue);
		for (ssize_t i = 1; i < m_width; ++i)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[m_width] = L'\0';
	}

	void PluginIntEditFieldBinding::save_() const
	{
		PCWSTR DataPtr = (PCWSTR)psi().SendDlgMessage(get_dlg(), DM_GETCONSTTEXTPTR, get_index(), 0);
		*Value = fsf().atoi(DataPtr);
	}

	ssize_t PluginIntEditFieldBinding::get_width() const
	{
		return m_width;
	}

	PWSTR PluginIntEditFieldBinding::GetBuffer()
	{
		return Buffer;
	}

	PCWSTR PluginIntEditFieldBinding::GetMask() const
	{
		return Mask;
	}

}
