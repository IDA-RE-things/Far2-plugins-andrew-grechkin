/**
	win_net

	@classes	()
	@author		© 2009 Andrew Grechkin
	@link		()
**/

#ifndef WIN_NET_HPP
#define WIN_NET_HPP

#include <winsock2.h>

#include <libwin_def/std.h>
#include <libwin_def/str.h>

///===================================================================================== FileVersion
struct FileVersion {
	FileVersion(PCWSTR path);

	ustring get_version() const {
		return m_ver;
	}

private:
	ustring m_ver;
};

///================================================================================== DinamicLibrary
class DynamicLibrary {
	typedef DynamicLibrary this_type;

public:
	~DynamicLibrary();

	DynamicLibrary(const ustring & path);

	DynamicLibrary(const this_type & rhs);

	this_type & operator =(const this_type & rhs);

	operator HMODULE() const {
		return m_hnd;
	}

	HMODULE handle() const {
		return m_hnd;
	}

	FARPROC get_function_nt(PCSTR name) const throw();

	FARPROC get_function(PCSTR name) const;

	void swap(this_type & rhs);

private:
	HMODULE m_hnd;
	ustring m_path;
};

bool is_admin();

ustring	get_token_user(HANDLE hToken);

#endif
