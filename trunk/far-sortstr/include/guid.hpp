/**
	sortstr: Sort strings in editor
	FAR3 plugin

	© 2013 Andrew Grechkin

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

#ifndef _GUID_HPP_
#define _GUID_HPP_

#include <windows.h>

// {5FE4D394-2FE5-453F-8DB9-D63C32E01D13}
DEFINE_GUID(PluginGuid,
0x5fe4d394, 0x2fe5, 0x453f, 0x8d, 0xb9, 0xd6, 0x3c, 0x32, 0xe0, 0x1d, 0x13);

// {8F24402E-6080-4497-84D2-EA60FB360815}
DEFINE_GUID(MenuGuid,
0x8f24402e, 0x6080, 0x4497, 0x84, 0xd2, 0xea, 0x60, 0xfb, 0x36, 0x8, 0x15);

// {A8680C83-196E-487F-BE4B-540EB63A8F07}
DEFINE_GUID(ConfigGuid,
0xa8680c83, 0x196e, 0x487f, 0xbe, 0x4b, 0x54, 0xe, 0xb6, 0x3a, 0x8f, 0x7);

// {BDB1509B-091E-4AA6-B9E4-7DF3027C1FD8}
DEFINE_GUID(DialogGuid,
0xbdb1509b, 0x91e, 0x4aa6, 0xb9, 0xe4, 0x7d, 0xf3, 0x2, 0x7c, 0x1f, 0xd8);

#endif
