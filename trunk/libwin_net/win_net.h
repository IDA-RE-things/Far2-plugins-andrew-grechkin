/**
	win_net
	@author © 2011 Andrew Grechkin
**/

#ifndef WIN_NET_HPP
#define WIN_NET_HPP

#include <libwin_def/std.h>
#include <libwin_def/str.h>

///========================================================================================= Command
/// Паттерн Command
struct Command_p {
	virtual ~Command_p() {
	}
	virtual bool Execute() const = 0;
};

struct NullCommand: public Command_p {
	virtual bool Execute() const {
		return true;
	}
};

///======================================================================================= Singleton
/// Паттерн Singleton
template <typename T>
struct Singleton_p : private T {
	typedef T implementation_type;
	typedef Singleton_p<T> this_type;
	static T & instance() {
		static this_type instance;
		return instance;
	}

private:
	~Singleton_p() {
	}

	Singleton_p() {
	}
};

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

	DynamicLibrary(PCWSTR path, DWORD flags = 0);

	DynamicLibrary(const this_type & rhs);

	this_type & operator =(const this_type & rhs);

	HMODULE get_hmodule() const {
		return m_hndl;
	}

	DWORD get_flags() const {
		return m_flags;
	}

	ustring get_path() const;

	FARPROC get_function(PCSTR name) const;

	FARPROC get_function_nt(PCSTR name) const throw();

	FARPROC operator [](PCSTR name) const throw() {
		return get_function_nt(name);
	}

	void swap(this_type & rhs);

private:
	HMODULE m_hndl;
	DWORD m_flags;
};

#endif
