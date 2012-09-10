#include <libfar3/plugin_i.hpp>

#include <libbase/logger.hpp>


namespace Far {

	///==================================================================================== Plugin_i
	Plugin_i::~Plugin_i() {
		LogTrace();
	}

	HANDLE Plugin_i::Open(const OpenInfo * /*Info*/) {
		LogTrace();
		return nullptr;
	}

	void Plugin_i::ClosePanel(const ClosePanelInfo * /*Info*/) {
		LogTrace();
	}

	void Plugin_i::Exit(const ExitInfo * /*Info*/) {
		LogTrace();
	}

}
