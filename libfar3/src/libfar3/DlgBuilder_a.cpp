#include <libfar3/dialog_builder.hpp>

namespace Far {

	DialogItemBinding_i::~DialogItemBinding_i()
	{
	}

	void DialogItemBinding_i::save() const
	{
		if (this)
			save_();
	}

}
