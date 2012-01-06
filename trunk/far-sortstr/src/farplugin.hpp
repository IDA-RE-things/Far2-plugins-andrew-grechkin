#ifndef FARPLUGIN_H
#define FARPLUGIN_H

#include <libwin_def/std.h>
#include <libwin_def/shared_ptr.h>

#ifndef FAR2
#	include <libwin_def/str.h>
#	include <API_far3/helper.hpp>
//#	include "helper.hpp"
#else
#	include <libwin_def/reg.h>
#	include <API_far2/helper.hpp>
//#	include "helper.hpp"
#endif

// main dialog parameters
enum {
	HEIGHT = 15,
	WIDTH = 60,

	indInv = 1,
	indCS,
	indNS,
	indWhsp = 5,
	indSelected = 7,
	indAsEmpty,
	indList = 11,
};

struct Options {
	static const size_t WHITESPACES_LEN = 32;

	size_t inv, cs, ns, sel, emp, op;

	Options();

#ifndef FAR2
	void load();
#else
	void load(const ustring & path);
#endif

	void get_parameters(const Far::Dialog & dlg);

	void save() const;

	bool is_whitespace(WCHAR ch) const;

	ustring get_whitespaces() const;

	size_t get_first_line() const;

	size_t get_total_lines() const;

	ssize_t get_block_type() const;

	void load_editor_info();

private:
#ifndef FAR2
	winstd::shared_ptr<Far::Settings_t> m_settings;
#else
	Register reg;
#endif

	ustring m_whitespaces;

	EditorInfo m_ei;
	mutable size_t m_first_line;
};

struct FarPlugin {
	FarPlugin(const PluginStartupInfo * psi);

	void get_info(PluginInfo * pi) const;

#ifndef FAR2
	HANDLE open(const OpenInfo * Info);

	static GUID get_guid();
#else
	HANDLE open(int OpenFrom, INT_PTR Item);
#endif

	bool Execute() const;

	static PCWSTR get_name();

	static PCWSTR get_description();

	static PCWSTR get_author();

private:
	long double FindNum(PCWSTR str) const;

	Options options;
};

extern winstd::shared_ptr<FarPlugin> plugin;

#endif
