#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_def/std.h>
#include <libwin_def/shared_ptr.h>

#ifndef FAR2
#	include <API_far3/helper.hpp>
#	include <libwin_def/str.h>
typedef Far::Settings_t Settings_type;
#else
#	include <API_far2/helper.hpp>
#	include <libwin_def/reg.h>
typedef Register Settings_type;
#endif

struct FarPlugin;
extern windef::shared_ptr<FarPlugin> plugin;

// main dialog parameters
enum {
	HEIGHT = 11,
	WIDTH = 70,

	indDelAll = 1,
	indDelRep,
	indDelWithText,
	indDelWithoutText,
	indText,
	indIsMask,
};

struct Options {
	size_t op, opm;
	ustring text;

	Options();

	void get_parameters(const Far::Dialog & dlg);

	void load();

	void save() const;

	size_t get_current_line() const;

	size_t get_current_column() const;

	size_t get_first_line() const;

	size_t get_total_lines() const;

	ssize_t get_block_type() const;

	void load_editor_info();

private:
	windef::shared_ptr<Settings_type> m_settings;

	EditorInfo m_ei;
	mutable size_t m_first_line;
};

struct FarPlugin {
	FarPlugin(const PluginStartupInfo * psi);

	bool Execute() const;

	void get_info(PluginInfo * pi) const;

#ifndef FAR2
	HANDLE open(const OpenInfo * Info);

	static GUID get_guid();
#else
	HANDLE open(int OpenFrom, INT_PTR Item);
#endif

	PCWSTR get_prefix() const;

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();

private:
	void delete_string(size_t & index, size_t & total, size_t & current) const;

	Options options;
};

#endif
