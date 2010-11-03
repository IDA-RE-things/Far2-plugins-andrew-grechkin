/**
 * win_wcout
 * @classes	()
 * @author	2010 Andrew Grechkin
 * @link	()
 **/

#ifndef WIN_WCOUT_HPP
#define WIN_WCOUT_HPP

#include "win_def.h"
#include <sstream>

namespace WinWcout {
	struct WideConsoleOut {
		template<typename Type>
		WideConsoleOut& operator<<(const Type &in) {
			m_str << in;
			if (consoleout(m_str.str()))
				m_str.str(L"");
			return *this;
		}
		WideConsoleOut& operator<<(const AutoUTF &in) {
			return this->operator<<(in.c_str());
		}
	private:
		std::wstringstream m_str;
	};

	PCWSTR endl = L"\n";
	WideConsoleOut wcout;
}

#endif // WIN_WCOUT_HPP
