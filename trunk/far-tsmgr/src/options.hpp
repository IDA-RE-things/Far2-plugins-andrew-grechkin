﻿/**
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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <libwin_def/reg.h>

PCWSTR const DEFAULT_PLUGIN_PREFIX = L"tsmgr";

struct	PluginOptions {
	int		AddToPluginsMenu;
	int		AddToDisksMenu;
	AutoUTF	Prefix;

	PluginOptions();
	void	Init(PCWSTR root);
	void	Read();
	void	Write() const;

private:
	Register	reg;
};

extern PluginOptions	Options;

#endif
