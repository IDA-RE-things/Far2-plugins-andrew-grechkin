#ifndef _WIN_SHLINK_H_
#define _WIN_SHLINK_H_

#include "win_com.h"

struct ShellIcon {
public:
	ustring path;
	int index;

	ShellIcon(const ustring &p, int i):
		path(p),
		index(i) {
	}
};

struct IShellLinkW;
class ShellLink {
public:
	~ShellLink();

	ShellLink(PCWSTR path, bool write = false);

	ShellLink(const ShellLink &rhs);

	ShellLink& operator=(const ShellLink &rhs);

	ustring args() const;

	ustring descr() const;

	ShellIcon icon() const;

	ustring path() const;

	int show() const;

	ustring work_dir() const;

	void args(const ustring &in);

	void descr(const ustring &in);

	void icon(const ShellIcon &in);

	void path(const ustring &in);

	void show(int in);

	void work_dir(const ustring &in);

	void write() const;

	static ShellLink create(PCWSTR path);

private:
	ShellLink();

	ustring m_path;
	ComObject<IShellLinkW> m_lnk;
};

#endif
