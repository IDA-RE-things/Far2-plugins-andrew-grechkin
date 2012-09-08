#include <panelmodel.hpp>

bool PanelModel::is_drivers() const {
	return get_type() == Ext::Service::TYPE_DRV;
}
