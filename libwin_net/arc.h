/** win_arc.hpp
 *	@functions to manipulate archives
 *	@author GrAnD, 2009
 *	@link (7-zip32)
**/

#ifndef WIN_ARC_HPP
#define WIN_ARC_HPP

#include <libwin_def/win_def.h>

#include <7-zip32.h>

namespace	SZip {
int		Compress(const astring &arc, const astring &path, const astring &type = "7z", const astring &lvl = "5" , bool recur = true);
int		Extract(const astring &arc, const astring &path, const astring &fl = "*", bool recur = true, bool fullpath = true);

int		Delete(const astring &arc, const astring &path);
}

#endif
