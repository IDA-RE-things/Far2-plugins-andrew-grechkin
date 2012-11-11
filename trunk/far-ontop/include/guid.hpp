/**
	ontop: Always on top FAR3 plugin
	Switch between "always on top" state on/off

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

// {8B0243FD-A66D-44E5-9825-38DA0AD451BD}
DEFINE_GUID(PluginGuid,
0x8b0243fd, 0xa66d, 0x44e5, 0x98, 0x25, 0x38, 0xda, 0xa, 0xd4, 0x51, 0xbd);

// {E12D319C-AA52-4A40-A4CC-E6394038BB11}
DEFINE_GUID(MenuGuid,
0xe12d319c, 0xaa52, 0x4a40, 0xa4, 0xcc, 0xe6, 0x39, 0x40, 0x38, 0xbb, 0x11);

// {66FB9AFC-70CF-4306-BDD9-CA88406F6664}
DEFINE_GUID(DialogGuid,
0x66fb9afc, 0x70cf, 0x4306, 0xbd, 0xd9, 0xca, 0x88, 0x40, 0x6f, 0x66, 0x64);

#endif
