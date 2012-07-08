/**
	svcmgr: Manage services
	Allow to manage windows services
	FAR3 plugin

	© 2012 Andrew Grechkin

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef _FAR_GUID_HPP_
#define _FAR_GUID_HPP_

#include <windows.h>

// {EB39ECBE-B2FF-4696-95D1-EFA7D3B88679}
DEFINE_GUID(PluginGuid,
0xeb39ecbe, 0xb2ff, 0x4696, 0x95, 0xd1, 0xef, 0xa7, 0xd3, 0xb8, 0x86, 0x79);

// {B528B932-778A-41B7-BC02-C6EF396E962D}
DEFINE_GUID(MenuGuid,
0xb528b932, 0x778a, 0x41b7, 0xbc, 0x2, 0xc6, 0xef, 0x39, 0x6e, 0x96, 0x2d);

// {14FB057A-5977-42E1-8E07-790369558C70}
DEFINE_GUID(DiskGuid,
0x14fb057a, 0x5977, 0x42e1, 0x8e, 0x7, 0x79, 0x3, 0x69, 0x55, 0x8c, 0x70);

// {FF3F0AC6-77EF-445B-8558-AFFE6D3572FE}
DEFINE_GUID(ConfigDialogGuid,
0xff3f0ac6, 0x77ef, 0x445b, 0x85, 0x58, 0xaf, 0xfe, 0x6d, 0x35, 0x72, 0xfe);

// {8A0FFA65-D911-4374-BBD8-6BF2D7A35A2D}
DEFINE_GUID(ConnectionDialogGuid,
0x8a0ffa65, 0xd911, 0x4374, 0xbb, 0xd8, 0x6b, 0xf2, 0xd7, 0xa3, 0x5a, 0x2d);

// {52CF442B-D8BA-4DF5-B4A2-99B9C133DB06}
DEFINE_GUID(CreateServiceDialogGuid,
0x52cf442b, 0xd8ba, 0x4df5, 0xb4, 0xa2, 0x99, 0xb9, 0xc1, 0x33, 0xdb, 0x6);

// {65E0C3D5-5B96-46A8-AF87-85D33040798B}
DEFINE_GUID(LogonAsDialogGuid,
0x65e0c3d5, 0x5b96, 0x46a8, 0xaf, 0x87, 0x85, 0xd3, 0x30, 0x40, 0x79, 0x8b);

// {B86B5F97-2CE5-46B9-A49C-53D75D1F7A93}
DEFINE_GUID(EditServiceDialogGuid,
0xb86b5f97, 0x2ce5, 0x46b9, 0xa4, 0x9c, 0x53, 0xd7, 0x5d, 0x1f, 0x7a, 0x93);

#endif
