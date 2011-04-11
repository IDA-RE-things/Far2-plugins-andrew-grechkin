#ifndef _WIN_SHLINK_H_
#define _WIN_SHLINK_H_

#include "win_com.h"


struct ShellIcon {
public:
	AutoUTF path;
	int index;

	ShellIcon(const AutoUTF &p, int i):
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

	AutoUTF args() const;

	AutoUTF descr() const;

	ShellIcon icon() const;

	AutoUTF path() const;

	int show() const;

	AutoUTF work_dir() const;

	void args(const AutoUTF &in);

	void descr(const AutoUTF &in);

	void icon(const ShellIcon &in);

	void path(const AutoUTF &in);

	void show(int in);

	void work_dir(const AutoUTF &in);

	void write() const;

	static ShellLink create(PCWSTR path);

private:
	ShellLink();

	AutoUTF m_path;
	ComObject<IShellLinkW> m_lnk;
};

#endif
