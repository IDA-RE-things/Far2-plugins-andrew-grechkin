#ifndef _WIN_DEF_LINK_H_
#define _WIN_DEF_LINK_H_

#include "win_def.h"

///============================================================================================ Link
bool is_link(PCWSTR path);
inline bool is_link(const AutoUTF &path) {
	return is_link(path.c_str());
}

bool is_symlink(PCWSTR path);
inline bool is_symlink(const AutoUTF &path) {
	return is_symlink(path.c_str());
}

bool is_junction(PCWSTR path);
inline bool is_junction(const AutoUTF &path) {
	return is_junction(path.c_str());
}

bool copy_link(PCWSTR from, PCWSTR to);
inline bool copy_link(const AutoUTF &from, const AutoUTF &to) {
	return copy_link(from, to);
}

bool create_link(PCWSTR link, PCWSTR dest);
inline bool	create_link(const AutoUTF &link, const AutoUTF &dest) {
	return create_link(link.c_str(), dest.c_str());
}

bool create_junc(PCWSTR path, PCWSTR dest);
inline bool	create_junc(const AutoUTF &path, const AutoUTF &dest) {
	return create_junc(path.c_str(), dest.c_str());
}

bool delete_link(PCWSTR path);
inline bool delete_link(const AutoUTF &path) {
	return delete_link(path.c_str());
}

bool break_link(PCWSTR path);
inline bool break_link(const AutoUTF &path) {
	return break_link(path.c_str());
}

AutoUTF	read_link(PCWSTR path);
inline AutoUTF read_link(const AutoUTF &path) {
	return read_link(path.c_str());
}

#endif
