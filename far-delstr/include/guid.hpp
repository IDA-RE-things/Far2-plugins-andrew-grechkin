/**
	delstr: Delete strings in editor
	FAR3lua plugin

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

// {AE501DD6-AD57-4A26-A5DF-39ACC3A9DB99}
DEFINE_GUID(PluginGuid,
0xae501dd6, 0xad57, 0x4a26, 0xa5, 0xdf, 0x39, 0xac, 0xc3, 0xa9, 0xdb, 0x99);

// {5B53782F-8A1F-413F-822A-EB7C16736279}
DEFINE_GUID(MenuGuid,
0x5b53782f, 0x8a1f, 0x413f, 0x82, 0x2a, 0xeb, 0x7c, 0x16, 0x73, 0x62, 0x79);

// {3090C558-18C9-4061-ABC0-5B8A1A9A15C4}
DEFINE_GUID(DialogGuid,
0x3090c558, 0x18c9, 0x4061, 0xab, 0xc0, 0x5b, 0x8a, 0x1a, 0x9a, 0x15, 0xc4);

#endif
