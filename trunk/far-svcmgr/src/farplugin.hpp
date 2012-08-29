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

#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libbase/std.hpp>
#include <libbase/shared_ptr.hpp>
#include <libbase/str.hpp>
#include <libbase/logger.hpp>
#include <libext/exception.hpp>
#include <libext/service.hpp>
#include <libext/rc.hpp>

#include <libfar3/helper.hpp>

#include <vector>

extern Base::shared_ptr<Far::Plugin_i> plugin;


///======================================================================================= FarPlugin
struct FarPlugin: public Far::Plugin_i {
	static FarPlugin * create(const PluginStartupInfo * psi);

	virtual void destroy();

	virtual void GetPluginInfo(PluginInfo * pi);

	virtual HANDLE Open(const OpenInfo * Info);

	virtual void Close(const ClosePanelInfo * Info);

	virtual int Configure(const ConfigureInfo * Info);

private:
	~FarPlugin();

	FarPlugin(const PluginStartupInfo * psi);
};


#endif
