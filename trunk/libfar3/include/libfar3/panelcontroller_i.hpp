#ifndef PANELCONTROLLER_HPP_
#define PANELCONTROLLER_HPP_

#include <libfar3/plugin.hpp>


namespace Far {

	///=========================================================================== PanelController_i
	struct PanelController_i {
		virtual ~PanelController_i();

		virtual void GetOpenPanelInfo(OpenPanelInfo * Info) = 0;

		virtual int GetFindData(GetFindDataInfo * Info) = 0;

		virtual void FreeFindData(const FreeFindDataInfo * Info) = 0;

		virtual int Compare(const CompareInfo * Info) = 0;

		virtual int SetDirectory(const SetDirectoryInfo * Info) = 0;

		virtual int ProcessEvent(const ProcessPanelEventInfo * Info) = 0;

		virtual int ProcessKey(INPUT_RECORD rec) = 0;

		INT_PTR update(bool keep_selection = true) const;

		INT_PTR redraw() const;
	};

}



#endif
