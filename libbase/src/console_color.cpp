#include <libbase/console.hpp>

namespace Base {

	ConsoleColor::~ConsoleColor() {
		restore();
	}

	ConsoleColor::ConsoleColor(WORD color) :
		m_color(0) {
		if (color && save())
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), color);
	}

	void ConsoleColor::restore() {
		if (m_color) {
			::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), m_color);
			m_color = 0;
		}
	}

	bool ConsoleColor::save() {
		CONSOLE_SCREEN_BUFFER_INFO tmp;
		if (::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &tmp)) {
			m_color = tmp.wAttributes;
		}
		return m_color;
	}

}
