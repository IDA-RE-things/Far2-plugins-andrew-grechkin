/**
	svcmgr: Manage services
	Allow to manage windows services

	© 2010  Andrew Grechkin

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

PluginOptions::PluginOptions(): Prefix(L"svcmgr") {
}
void		PluginOptions::Read() {
	reg.Get(L"AddToPluginsMenu", AddToPluginsMenu, true);
	reg.Get(L"AddToDisksMenu", AddToDisksMenu, false);
	reg.Get(L"DiskMenuDigit", DiskMenuDigit, L' ');
	reg.Get(L"Timeout", TimeOut, 30000);
	reg.Get(L"Prefix", Prefix, Prefix);
	Check();
}
void		PluginOptions::Write() {
	Check();
	reg.Set(L"AddToPluginsMenu", AddToPluginsMenu);
	reg.Set(L"AddToDisksMenu", AddToDisksMenu);
	reg.Set(L"DiskMenuDigit", DiskMenuDigit);
	reg.Set(L"Timeout", TimeOut);
	reg.Set(L"Prefix", Prefix.c_str());
}
void		PluginOptions::Check() {
	TimeOut  = Min(Max(TimeOut, 1000), 99000);
}
