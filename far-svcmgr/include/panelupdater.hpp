#ifndef _FAR_PANELUPDATER_HPP_
#define _FAR_PANELUPDATER_HPP_

#include <libbase/observer_p.hpp>


struct PanelController;
struct PanelModel;


struct PanelUpdater: public Base::Observer_p {
	PanelUpdater(PanelController * cont, PanelModel * model);

	/// Base::Observer_p interface
	void notify(void * /*data*/);

private:
	PanelController * m_controller;
};


#endif
