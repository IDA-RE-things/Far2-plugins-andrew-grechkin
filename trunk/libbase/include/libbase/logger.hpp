#ifndef _LIBBASE_LOGGER_HPP_
#define _LIBBASE_LOGGER_HPP_

#include <libbase/std.hpp>

namespace Base {
	namespace Logger {

		enum Level {
			LVL_TRACE,
			LVL_DEBUG,
			LVL_INFO,
			LVL_REPORT,
			LVL_ATTEN,
			LVL_WARN,
			LVL_ERROR,
			LVL_FATAL,

			LVL_END__,
		};

		enum Wideness {
			WIDE_FULL,
			WIDE_MODULE,
			WIDE_MEDIUM,
			WIDE_SHORT,

			WIDE_END__,
		};

		const Level defaultLevel = LVL_WARN;
		const Wideness defaultWideness = WIDE_MEDIUM;

		PCWSTR const defaultModule = L"default";

		struct Logger_i;

		///==================================================================================== Target_i
		struct Target_i {
			virtual ~Target_i() = 0;

			virtual void out(const Logger_i * lgr, Level lvl, PCWSTR str, size_t size) const = 0;
		};

		Target_i * get_TargetToConsole();

		Target_i * get_TargetToFile(PCWSTR path);

		Target_i * get_TargetToSys();

		///==================================================================================== Logger_i
		struct Logger_i {
			virtual ~Logger_i() = 0;

			virtual Level get_level() const = 0;

			virtual Wideness get_wideness() const = 0;

			virtual void set_level(Level lvl) = 0;

			virtual void set_wideness(Wideness wide) = 0;

			virtual void set_color_mode(bool mode) = 0;

			virtual bool is_color_mode() const = 0;

			virtual void set_target(Target_i * target) = 0;

			virtual void out(PCSTR file, int line, PCSTR func, Level lvl, PCWSTR module, PCWSTR format, ...) const = 0;

			virtual void out(Level lvl, PCWSTR module, PCWSTR format, ...) const = 0;
		};

		Logger_i & get_instance();

		void init(Target_i * target, Level lvl = defaultLevel);

		void set_level(Level lvl);

		void set_wideness(Wideness wide);

		void set_color_mode(bool mode);

	}
}

#ifdef NDEBUG
#define LogTrace()
#define LogDebug(format, args ...)
#else
#define LogTrace()	Logger::get_instance().out(THIS_PLACE, Logger::LVL_TRACE, Logger::defaultModule, L"\n")
#define LogDebug(format, args ...)	Logger::get_instance().out(THIS_PLACE, Logger::LVL_DEBUG, Logger::defaultModule, format, ##args)
#endif
#define LogInfo(format, args ...)	Logger::get_instance().out(Logger::LVL_INFO, Logger::defaultModule, format, ##args)
#define LogReport(format, args ...)	Logger::get_instance().out(Logger::LVL_REPORT, Logger::defaultModule, format, ##args)
#define LogAtten(format, args ...)	Logger::get_instance().out(Logger::LVL_ATTEN, Logger::defaultModule, format, ##args)
#define LogWarn(format, args ...)	Logger::get_instance().out(Logger::LVL_WARN, Logger::defaultModule, format, ##args)
#define LogError(format, args ...)	Logger::get_instance().out(Logger::LVL_ERROR, Logger::defaultModule, format, ##args)
#define LogFatal(format, args ...)	Logger::get_instance().out(Logger::LVL_FATAL, Logger::defaultModule, format, ##args)

#endif
