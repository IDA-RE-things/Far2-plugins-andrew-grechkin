/**
 * win_wcout
 * @classes	()
 * @author	2010 Andrew Grechkin
 * @link	()
 **/

#ifndef WIN_WCOUT_HPP
#define WIN_WCOUT_HPP

#include <win_com.h>
#include <win_def.h>
#include <sstream>

namespace winstd {
	struct WideConsoleOut {
		template<typename Type>
		WideConsoleOut& operator<<(const Type &in) {
			m_str << in;
			if (consoleout(m_str.str().c_str(), m_str.str().size()))
				m_str.str(L"");
			return *this;
		}
		WideConsoleOut& operator<<(const AutoUTF &in) {
			return this->operator<<(in.c_str());
		}
		WideConsoleOut& operator<<(const BStr &in) {
			return this->operator<<((PCWSTR) in);
		}
		WideConsoleOut& operator<<(const Variant &in) {
			if (in.is_int()) {
				return this->operator<<(in.as_int());
			}
			if (in.is_uint()) {
				return this->operator<<(in.as_uint());
			}
			if (in.is_str()) {
				return this->operator<<(in.as_str());
			}
			if (in.is_empty()) {
				return this->operator<<(L"(empty)");
			}
			if (in.is_null()) {
				return this->operator<<(L"(null)");
			}
			return *this;
		}
		WideConsoleOut& operator<<(const PropVariant &in) {
			if (in.is_bool()) {
				return this->operator<<(in.as_bool());
			}
			if (in.is_int()) {
				return this->operator<<(in.as_int());
			}
			if (in.is_uint()) {
				return this->operator<<(in.as_uint());
			}
			if (in.is_str()) {
				return this->operator<<(in.as_str());
			}
			if (in.is_empty()) {
				return this->operator<<(L"(empty)");
			}
			if (in.is_null()) {
				return this->operator<<(L"(null)");
			}
			return *this;
		}
	private:
		std::wstringstream m_str;
	} wcout;

	PCWSTR endl = L"\n";
}

#endif // WIN_WCOUT_HPP
