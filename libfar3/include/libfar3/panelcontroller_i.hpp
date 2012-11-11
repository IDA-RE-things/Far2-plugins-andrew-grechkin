#ifndef PANELCONTROLLER_HPP_
#define PANELCONTROLLER_HPP_

#include <libfar3/plugin.hpp>
#include <libfar3/fwd.hpp>

namespace Far {

	///=========================================================================== PanelController_i
	struct PanelController_i {
	public:
		// nvi
		void ClosePanelW(const ClosePanelInfo * Info);

		ssize_t CompareW(const CompareInfo * Info);

		ssize_t DeleteFilesW(const DeleteFilesInfo * Info);

		void FreeFindDataW(const FreeFindDataInfo * Info);

		ssize_t GetFilesW(GetFilesInfo * Info);

		ssize_t GetFindDataW(GetFindDataInfo * Info);

		void GetOpenPanelInfoW(OpenPanelInfo * Info);

		ssize_t MakeDirectoryW(MakeDirectoryInfo * Info);

		ssize_t ProcessPanelEventW(const ProcessPanelEventInfo * Info);

		ssize_t ProcessHostFileW(const ProcessHostFileInfo * Info);

		ssize_t ProcessPanelInputW(const ProcessPanelInputInfo * Info);

		ssize_t PutFilesW(const PutFilesInfo * Info);

		ssize_t SetDirectoryW(const SetDirectoryInfo * Info);

		ssize_t SetFindListW(const SetFindListInfo * Info);

		INT_PTR update(bool keep_selection = true) const;

		INT_PTR redraw() const;

	public:
		PanelController_i();

		virtual ~PanelController_i();

	private:
		virtual void Close(const ClosePanelInfo * Info);

		virtual ssize_t Compare(const CompareInfo * Info);

		virtual ssize_t DeleteFiles(const DeleteFilesInfo * Info);

		virtual void FreeFindData(const FreeFindDataInfo * Info);

		virtual ssize_t GetFiles(GetFilesInfo * Info);

		virtual ssize_t GetFindData(GetFindDataInfo * Info);

		virtual void GetOpenPanelInfo(OpenPanelInfo * Info) = 0;

		virtual ssize_t MakeDirectory(MakeDirectoryInfo * Info);

		virtual ssize_t ProcessEvent(const ProcessPanelEventInfo * Info);

		virtual ssize_t ProcessInput(const ProcessPanelInputInfo * Info);

		virtual ssize_t ProcessHostFile(const ProcessHostFileInfo * Info);

		virtual ssize_t PutFiles(const PutFilesInfo * Info);

		virtual ssize_t SetDirectory(const SetDirectoryInfo * Info);

		virtual ssize_t SetFindList(const SetFindListInfo * Info);

//	private:
//		Plugin_i & m_plugin;
	};

}

#endif
