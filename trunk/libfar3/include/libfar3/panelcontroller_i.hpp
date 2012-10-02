#ifndef PANELCONTROLLER_HPP_
#define PANELCONTROLLER_HPP_

#include <libfar3/plugin.hpp>


namespace Far {


	struct GlobalInfo_i;
	struct PanelController_i;
	struct Plugin_i;


	///=========================================================================== PanelController_i
	struct PanelController_i {
	public: // nvi
		void ClosePanelW(const ClosePanelInfo * Info);

		int CompareW(const CompareInfo * Info);

		int DeleteFilesW(const DeleteFilesInfo * Info);

		void FreeFindDataW(const FreeFindDataInfo * Info);

		void FreeVirtualFindDataW(const FreeFindDataInfo * Info);

		int GetFilesW(GetFilesInfo * Info);

		int GetFindDataW(GetFindDataInfo * Info);

		void GetOpenPanelInfoW(OpenPanelInfo * Info);

		int GetVirtualFindDataW(GetVirtualFindDataInfo * Info);

		int MakeDirectoryW(MakeDirectoryInfo * Info);

		int ProcessPanelEventW(const ProcessPanelEventInfo * Info);

		int ProcessHostFileW(const ProcessHostFileInfo * Info);

		int ProcessPanelInputW(const ProcessPanelInputInfo * Info);

		int PutFilesW(const PutFilesInfo * Info);

		int SetDirectoryW(const SetDirectoryInfo * Info);

		int SetFindListW(const SetFindListInfo * Info);

		INT_PTR update(bool keep_selection = true) const;

		INT_PTR redraw() const;

	public:
		PanelController_i();

		virtual ~PanelController_i();

	private:
		virtual void Close(const ClosePanelInfo * Info);

		virtual int Compare(const CompareInfo * Info);

		virtual int DeleteFiles(const DeleteFilesInfo * Info);

		virtual void FreeFindData(const FreeFindDataInfo * Info);

		virtual void FreeVirtualFindData(const FreeFindDataInfo * Info);

		virtual int GetFiles(GetFilesInfo * Info);

		virtual int GetFindData(GetFindDataInfo * Info);

		virtual void GetOpenPanelInfo(OpenPanelInfo * Info) = 0;

		virtual int GetVirtualFindData(GetVirtualFindDataInfo * Info);

		virtual int MakeDirectory(MakeDirectoryInfo * Info);

		virtual int ProcessEvent(const ProcessPanelEventInfo * Info);

		virtual int ProcessInput(const ProcessPanelInputInfo * Info);

		virtual int ProcessHostFile(const ProcessHostFileInfo * Info);

		virtual int PutFiles(const PutFilesInfo * Info);

		virtual int SetDirectory(const SetDirectoryInfo * Info);

		virtual int SetFindList(const SetFindListInfo * Info);

//	private:
//		Plugin_i & m_plugin;
	};

}



#endif
