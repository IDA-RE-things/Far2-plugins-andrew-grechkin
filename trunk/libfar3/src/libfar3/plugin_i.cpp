#include <libfar3/plugin_i.hpp>
#include <libfar3/helper.hpp>

#include <libbase/logger.hpp>


namespace Far {

	///========================================================================================= nvi
	void Plugin_i::GetPluginInfoW(PluginInfo * Info) {
		Info->StructSize = sizeof(*Info);
		GetInfo(Info);
	}

	PanelController_i * Plugin_i::OpenW(const OpenInfo * Info) {
		if (Info->StructSize < sizeof(*Info))
			return nullptr;
		return Open(Info);
	}

	void Plugin_i::ExitFARW(const ExitInfo * Info) {
		if (Info->StructSize < sizeof(*Info))
			return;
		return Exit(Info);
	}

	const PluginStartupInfo & Plugin_i::psi() const {
		return m_psi;
	}

	const FarStandardFunctions & Plugin_i::fsf() const {
		return m_fsf;
	}


	///=============================================================================================
	Plugin_i::Plugin_i(GlobalInfo_i * gi, const PluginStartupInfo * Info)
	{
		LogTrace();
		m_psi = *Info;
		m_fsf = *Info->FSF;
		m_psi.FSF = &m_fsf;
		Far::helper_t::inst().init(gi, this);
	}

	Plugin_i::~Plugin_i() {
		LogTrace();
	}


	///=============================================================================================
	PanelController_i * Plugin_i::Open(const OpenInfo * /*Info*/) {
		LogTrace();
		return nullptr;
	}

	void Plugin_i::Exit(const ExitInfo * /*Info*/) {
		LogTrace();
	}

}
