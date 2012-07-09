#include "log.h"
#include "va_list.h"

#include <libwin_def/console.h>

namespace windef {
	PCWSTR LogLevelNames[LOG_LEVEL_END__] = {
		L"TRACE ",
		L"DEBUG ",
		L"INFO  ",
		L"WARN  ",
		L"ERROR ",
		L"FATAL ",
	};

	WORD LogLevelColors[LOG_LEVEL_END__] = {
		0,
		0,
		0,
		0,
		14,
		4,
	};

	PCWSTR loggerFormatStr = L"%s%S: %d [%S] ";
	PCWSTR loggerFormatStrShort = L"%s ";

	///==================================================================================== Target_i
	Target_i::~Target_i() {
	}

//	///===================================================================================== LogLine
//	struct LogLine {
//		~LogLine() {
//			if (!(m_lvl < m_log.get_level())) {
//				m_log.write(*m_oss);
//			}
//		}
//
//		LogLine(LogLevel lvl, const Logger & windef):
//			m_lvl(lvl),
//			m_log(windef),
//			m_oss(new ostringstream(ostringstream::out)) {
//		}
//
//
//		template<typename Type>
//		ostream& operator<<(const Type& in) {
//			if (!(m_lvl < m_log.getLogLevel()))
//				return (*m_oss) << in;
//			return *m_oss;
//		}
//
//	private:
//		LogLevel m_lvl;
//		const Logger &m_log;
//		shared_ptr<ostringstream> m_oss;
//	};
//
	///================================================================================== LoggetImpl
	Logger & Logger::inst() {
		static Logger ret;
		return ret;
	}

	Logger::~Logger() {
	}

	Logger::Logger():
		m_lvl(defaultLevel) {
	}

	LogLevel Logger::get_level() const {
		return m_lvl;
	}

	void Logger::set_level(LogLevel lvl) {
		m_lvl = lvl;
	}

	void Logger::set_color_mode(bool in) {
		m_color = in;
	}

	bool Logger::is_color_mode() const {
		return m_color;
	}

	void Logger::set_target(Target_i * target) {
		m_target.reset(target);
	}

	void Logger::out(LogLevel lvl, PCWSTR format, ...) const {
		if (lvl >= m_lvl) {
			va_list args;
			va_start(args, format);
			ustring tmp = windef::vargs_as_str(loggerFormatStrShort, LogLevelNames[lvl]);
			out_args(tmp, format, args);
		}
	}

	void Logger::out(LogLevel lvl, PCSTR file, int line, PCSTR func, PCWSTR format, ...) const {
		if (lvl >= m_lvl) {
			va_list args;
			va_start(args, format);
			ustring tmp = windef::vargs_as_str(loggerFormatStr, LogLevelNames[lvl], file, line, func);
			out_args(tmp, format, args);
		}
	}

	void Logger::out_args(const ustring & prefix, PCWSTR format, va_list args) const {
		ustring tmp(prefix);
		tmp += windef::va_list_as_str(format, args);
		m_target->out(tmp);
	}

	///=================================================================================================
//	LogType InitializeLog(LogMethod type, const String &str) {
//		LogType lg;
//		if (type == LOG_TO_CON)
//			lg.reset(new LogCon);
//		else if (type == LOG_TO_FILE)
//			lg.reset(new LogFile(str));
//		return lg;
//	}
//
//	void InitLog(LogLevel lvl, LogMethod type, const String &str) {
//		gLOG = InitializeLog(type, str);
//		LogInfo << "Log initialized: " << __DATE__ << " " << __TIME__;
//		gLOG->setLogLevel(lvl);
//	}
//
//	ostream& operator <<(ostream& ost, const Exception & exc) {
//		return ost << exc.getType() << " " << exc.getCode() << ": " << exc.getMessage();
//	}
}


///==================================================================================== LogToConsole
LogToConsole::~LogToConsole() {
}

LogToConsole::LogToConsole() {
}

void LogToConsole::out(const ustring & str) const {
	consoleout(str.c_str());
}


///======================================================================================= LogToFile
LogToFile::~LogToFile() {
	::CloseHandle(m_file);
}

LogToFile::LogToFile(const ustring & path)
{
	m_file = ::CreateFileW(path.c_str(),
	                       GENERIC_WRITE, FILE_SHARE_READ, nullptr,
	                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_file) {
		::SetFilePointer(m_file, 0,nullptr, FILE_END);
	}
}

void LogToFile::out(const ustring & str) const {
	windef::fileout(m_file, str.c_str(), str.size());
}

