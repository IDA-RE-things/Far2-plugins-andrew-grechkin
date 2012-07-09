#ifndef _WIN_DEF_LOG_HPP
#define _WIN_DEF_LOG_HPP

#include "win_def.h"
#include "shared_ptr.h"

enum LogLevel {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_LEVEL_END__,
};

namespace windef {
	///==================================================================================== Target_i
	struct Target_i {
		virtual ~Target_i();

		virtual void out(const ustring & str) const = 0;
	};

	///====================================================================================== Logger
	struct LoggerImpl;

	struct Logger {
		static const LogLevel defaultLevel = LOG_WARN;

		static Logger & inst();

		~Logger();

		LogLevel get_level() const;

		void set_level(LogLevel lvl);

		void set_color_mode(bool in);

		bool is_color_mode() const;

		void set_target(Target_i * target);

		void out(LogLevel lvl, PCWSTR format, ...) const;

		void out(LogLevel lvl, PCSTR file, int line, PCSTR func, PCWSTR format, ...) const;

	private:
		Logger();

		void out_args(const ustring & prefix, PCWSTR format, va_list args) const;

		windef::shared_ptr<Target_i> m_target;

		LogLevel m_lvl;

		bool m_color;
	};

}


///=================================================================================================
inline void logger_init(windef::Target_i * target, LogLevel lvl = LOG_WARN) {
	windef::Logger::inst().set_target(target);
	windef::Logger::inst().set_level(lvl);
}

inline void logger_set_level(LogLevel lvl) {
	windef::Logger::inst().set_level(lvl);
}


///==================================================================================== LogToConsole
struct LogToConsole: public windef::Target_i {
	~LogToConsole();

	LogToConsole();

	void out(const ustring & str) const;
};


///======================================================================================= LogToFile
struct LogToFile: public windef::Target_i {
	~LogToFile();

	LogToFile(const ustring & path);

	void out(const ustring & str) const;

private:
	HANDLE m_file;
};


///======================================================================================== LogToSys
struct LogToSys: public windef::Target_i {
	~LogToSys();

	LogToSys(const ustring & path);

	void out(const ustring & str) const;

private:
};


#ifdef NDEBUG
#define LogTrace()
#define LogDebug(format, args ...)
#else
#define LogTrace()	windef::Logger::inst().out(LOG_TRACE, THROW_PLACE, L"\n")
#define LogDebug(format, args ...)	windef::Logger::inst().out(LOG_DEBUG, THROW_PLACE, format, ##args)
#endif
#define LogInfo(format, args ...)	windef::Logger::inst().out(LOG_INFO, format, ##args)
#define LogWarn(format, args ...)	windef::Logger::inst().out(LOG_WARN, format, ##args)
#define LogError(format, args ...)	windef::Logger::inst().out(LOG_ERROR, format, ##args)
#define LogFatal(format, args ...)	windef::Logger::inst().out(LOG_FATAL, format, ##args)


#endif
