#ifndef PLUGIN_I_HPP_
#define PLUGIN_I_HPP_

#include <libfar3/plugin.hpp>


namespace Far {

	///==================================================================================== Plugin_i
	struct Plugin_i {
		virtual ~Plugin_i();

		//Plugin_i(const PluginStartupInfo * psi);

		virtual void GetPluginInfo(PluginInfo * pi) = 0;

		virtual HANDLE Open(const OpenInfo * Info);

		virtual void ClosePanel(const ClosePanelInfo * Info);

		virtual void Exit(const ExitInfo * Info);
	};

}


#endif
