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

#ifndef _FAR_GLOBALINFO_HPP_
#define _FAR_GLOBALINFO_HPP_

#include <libfar3/globalinfo_i.hpp>
#include <libfar3/settings.hpp>

#include <libbase/shared_ptr.hpp>


///====================================================================================== GlobalInfo
struct FarGlobalInfo: public Far::GlobalInfo_i, private Base::Uncopyable {
	FarGlobalInfo();

	~FarGlobalInfo();

	virtual GUID get_guid() const;

	virtual PCWSTR get_name() const;

	virtual PCWSTR get_description() const;

	virtual PCWSTR get_author() const;

	virtual VersionInfo get_version() const;

	virtual int Configure(const ConfigureInfo * Info);

	void load_settings();

	ssize_t AddToPluginsMenu;
	ssize_t AddToDisksMenu;
	WCHAR Prefix[32];

private:
	void save_settings() const;

	Far::Settings_t * m_settings;
};


extern Base::shared_ptr<FarGlobalInfo> globalInfo;


FarGlobalInfo * create_GlobalInfo();

void destroy(FarGlobalInfo * info);


#endif
