/**
	filever: File Version FAR plugin
	Displays version information from file resource in dialog
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

// {F9AF80C3-54AC-4924-B607-FE05A3866A88}
DEFINE_GUID(PluginGuid,
0xf9af80c3, 0x54ac, 0x4924, 0xb6, 0x7, 0xfe, 0x5, 0xa3, 0x86, 0x6a, 0x88);

// {736D23A8-E1F1-41E1-ACD8-AB6B49EE4CD5}
DEFINE_GUID(MenuGuid,
0x736d23a8, 0xe1f1, 0x41e1, 0xac, 0xd8, 0xab, 0x6b, 0x49, 0xee, 0x4c, 0xd5);

// {93F0F6D0-78F3-4208-ABC6-0457506AA2CC}
DEFINE_GUID(DialogGuid,
0x93f0f6d0, 0x78f3, 0x4208, 0xab, 0xc6, 0x4, 0x57, 0x50, 0x6a, 0xa2, 0xcc);

#endif
