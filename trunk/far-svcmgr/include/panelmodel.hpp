#ifndef _FAR_PANELMODEL_HPP_
#define _FAR_PANELMODEL_HPP_

#include <libext/services.hpp>


struct PanelModel: public Ext::Services {
	bool is_drivers() const;
};


#endif
