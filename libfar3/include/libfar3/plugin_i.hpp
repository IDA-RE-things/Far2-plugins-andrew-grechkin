#ifndef _FAR_PLUGIN_I_HPP_
#define _FAR_PLUGIN_I_HPP_

#include <libfar3/plugin.hpp>
#include <libfar3/fwd.hpp>

namespace Far {

	///==================================================================================== Plugin_i
	struct Plugin_i {
	public:
		Plugin_i(GlobalInfo_i * gi, const PluginStartupInfo * Info);

		virtual ~Plugin_i();

	public: // nvi
		void GetPluginInfoW(PluginInfo * Info);

		PanelController_i * OpenW(const OpenInfo * Info);

		void ExitFARW(const ExitInfo * Info);

		const PluginStartupInfo & psi() const;

		const FarStandardFunctions & fsf() const;

	private:
		virtual void GetInfo(PluginInfo * pi) = 0;

		virtual PanelController_i * Open(const OpenInfo * Info);

		virtual void Exit(const ExitInfo * Info);

		PluginStartupInfo m_psi;
		FarStandardFunctions m_fsf;
	};

}

#endif