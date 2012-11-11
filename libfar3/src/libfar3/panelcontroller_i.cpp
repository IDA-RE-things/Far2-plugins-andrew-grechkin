#include <libfar3/panelcontroller_i.hpp>
#include <libfar3/helper.hpp>

#include <libbase/logger.hpp>

namespace Far {

	///========================================================================================= nvi
	void PanelController_i::ClosePanelW(const ClosePanelInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return;
		Close(Info);
	}

	ssize_t PanelController_i::CompareW(const CompareInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return -2;
		return Compare(Info);
	}

	ssize_t PanelController_i::DeleteFilesW(const DeleteFilesInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return DeleteFiles(Info);
	}

	void PanelController_i::FreeFindDataW(const FreeFindDataInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return;
		FreeFindData(Info);
	}

	ssize_t PanelController_i::GetFilesW(GetFilesInfo * Info)
	{
		Info->StructSize = sizeof(*Info);
		return GetFiles(Info);
	}

	ssize_t PanelController_i::GetFindDataW(GetFindDataInfo * Info)
	{
		Info->StructSize = sizeof(*Info);
		return GetFindData(Info);
	}

	void PanelController_i::GetOpenPanelInfoW(OpenPanelInfo * Info)
	{
		Info->StructSize = sizeof(*Info);
		GetOpenPanelInfo(Info);
	}

	ssize_t PanelController_i::MakeDirectoryW(MakeDirectoryInfo * Info)
	{
		Info->StructSize = sizeof(*Info);
		return MakeDirectory(Info);
	}

	ssize_t PanelController_i::ProcessPanelEventW(const ProcessPanelEventInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return ProcessEvent(Info);
	}

	ssize_t PanelController_i::ProcessHostFileW(const ProcessHostFileInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return ProcessHostFile(Info);
	}

	ssize_t PanelController_i::ProcessPanelInputW(const ProcessPanelInputInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return ProcessInput(Info);
	}

	ssize_t PanelController_i::PutFilesW(const PutFilesInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return PutFiles(Info);
	}

	ssize_t PanelController_i::SetDirectoryW(const SetDirectoryInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return SetDirectory(Info);
	}

	ssize_t PanelController_i::SetFindListW(const SetFindListInfo * Info)
	{
		if (Info->StructSize < sizeof(*Info))
			return 0;
		return SetFindList(Info);
	}

	INT_PTR PanelController_i::update(bool keep_selection) const
	{
		return psi().PanelControl((HANDLE)this, FCTL_UPDATEPANEL, keep_selection, nullptr);
	}

	INT_PTR PanelController_i::redraw() const
	{
		return psi().PanelControl((HANDLE)this, FCTL_REDRAWPANEL, 0, nullptr);
	}

	///============================================================================================
	PanelController_i::PanelController_i()
	{
	}

	PanelController_i::~PanelController_i()
	{
	}

	void PanelController_i::Close(const ClosePanelInfo * /*Info*/)
	{
		delete this;
	}

	ssize_t PanelController_i::Compare(const CompareInfo * /*Info*/)
	{
		return -2;
	}

	ssize_t PanelController_i::DeleteFiles(const DeleteFilesInfo * /*Info*/)
	{
		return 0;
	}

	void PanelController_i::FreeFindData(const FreeFindDataInfo * /*Info*/)
	{
	}

	ssize_t PanelController_i::GetFiles(GetFilesInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::GetFindData(GetFindDataInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::MakeDirectory(MakeDirectoryInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::ProcessEvent(const ProcessPanelEventInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::ProcessInput(const ProcessPanelInputInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::ProcessHostFile(const ProcessHostFileInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::PutFiles(const PutFilesInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::SetDirectory(const SetDirectoryInfo * /*Info*/)
	{
		return 0;
	}

	ssize_t PanelController_i::SetFindList(const SetFindListInfo * /*Info*/)
	{
		return 0;
	}
}

