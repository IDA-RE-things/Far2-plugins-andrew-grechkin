/**
	tsmgr: Terminal sessions manager FAR plugin
	Allow to manage sessions on Terminal server

	© 2010 Andrew Grechkin

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

#include "options.hpp"

///======================================================================================= implement
PluginOptions			Options;

PluginOptions::PluginOptions(): Prefix(L"tsmgr") {
	AddToPluginsMenu = true;
	AddToDisksMenu = false;
	DiskMenuDigit = L' ';
}
void		PluginOptions::Read() {
	reg.Get(L"AddToPluginsMenu", AddToPluginsMenu, true);
	reg.Get(L"AddToDisksMenu", AddToDisksMenu, false);
	reg.Get(L"DiskMenuDigit", DiskMenuDigit, L' ');
	reg.Get(L"Prefix", Prefix, Prefix);
}
void		PluginOptions::Write() {
	reg.Set(L"AddToPluginsMenu", AddToPluginsMenu);
	reg.Set(L"AddToDisksMenu", AddToDisksMenu);
	reg.Set(L"DiskMenuDigit", DiskMenuDigit);
	reg.Set(L"Prefix", Prefix);
}
