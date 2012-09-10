#include <libfar3/panelcontroller_i.hpp>
#include <libfar3/helper.hpp>

#include <libbase/std.hpp>


namespace Far {

	///===================================================================================== Panel_i
	PanelController_i::~PanelController_i() {
	}

	INT_PTR PanelController_i::update(bool keep_selection) const {
		return psi().PanelControl((HANDLE)this, FCTL_UPDATEPANEL, keep_selection, nullptr);
	}

	INT_PTR PanelController_i::redraw() const {
		return psi().PanelControl((HANDLE)this, FCTL_REDRAWPANEL, 0, nullptr);
	}

}
