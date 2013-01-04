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

#ifndef _FAR_GLOBALINFO_HPP_
#define _FAR_GLOBALINFO_HPP_

#include <libfar3/globalinfo_i.hpp>
#include <libbase/std.hpp>
#include <libbase/uncopyable.hpp>

struct FarGlobalInfo: public Far::GlobalInfo_i, private Base::Uncopyable {
	~FarGlobalInfo();

	FarGlobalInfo();

	PCWSTR get_author() const override;

	PCWSTR get_description() const override;

	const GUID * get_guid() const override;

	PCWSTR get_title() const override;

	VersionInfo get_min_version() const override;

	Far::Plugin_i * CreatePlugin(const PluginStartupInfo * Info) const override;

	void load_settings();

	void save_settings() const;

	void load_editor_info();

	size_t get_first_line() const
	{
		return m_first_line;
	}

	size_t get_total_lines() const
	{
		return m_ei.TotalLines;
	}

	ssize_t get_block_type() const
	{
		return m_ei.BlockType;
	}

	ssize_t cbValue_Invert;
	ssize_t cbValue_Sensitive;
	ssize_t cbValue_Numeric;
	ssize_t cbValue_Selected;
	ssize_t cbValue_AsEmpty;
	WCHAR edValue_Whitespaces[32];
	WCHAR prefix[32];

	ssize_t cbValue_Operation;

private:
	// runtime info
	EditorInfo m_ei;
	mutable size_t m_first_line;
};

FarGlobalInfo * get_global_info();

#endif
