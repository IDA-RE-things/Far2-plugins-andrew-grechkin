/**
 © 2012 Andrew Grechkin
 Source code: <http://code.google.com/p/andrew-grechkin>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef _LIBFAR_PANELCONTROLLER_HPP_
#define _LIBFAR_PANELCONTROLLER_HPP_

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
