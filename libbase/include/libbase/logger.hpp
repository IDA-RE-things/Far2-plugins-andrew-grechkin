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
		};

		enum Wideness {
			WIDE_FULL,
			WIDE_MODULE,
			WIDE_MEDIUM,
			WIDE_SHORT,
		};

		const Level defaultLevel = LVL_WARN;
		const Wideness defaultWideness = WIDE_MEDIUM;

		PCWSTR const defaultModule = L"default";

		struct Module_i;

		///================================================================================ Target_i
		struct Target_i {
			virtual ~Target_i() = 0;

			virtual void out(const Module_i * module, Level lvl, PCWSTR str, size_t size) const = 0;
		};


		Target_i * get_TargetToConsole();

		Target_i * get_TargetToFile(PCWSTR path);

		Target_i * get_TargetToSys(PCWSTR name, PCWSTR path = nullptr);

		///================================================================================ Module_i
		struct Module_i {
			virtual ~Module_i() = 0;

			virtual PCWSTR get_name() const = 0;

			virtual Level get_level() const = 0;

			virtual Wideness get_wideness() const = 0;

			virtual void set_level(Level lvl) = 0;

			virtual void set_wideness(Wideness wide) = 0;

			virtual void set_color_mode(bool mode) = 0;

			virtual bool is_color_mode() const = 0;

			virtual void set_target(Target_i * target) = 0;

			virtual void out(PCSTR file, int line, PCSTR func, Level lvl, PCWSTR format, ...) const = 0;

			virtual void out(Level lvl, PCWSTR format, ...) const = 0;
		};

		///================================================================================ Logger_i
		struct Logger_i {
			virtual ~Logger_i() = 0;

			Module_i & operator [](PCWSTR module) const;

			void add_module(PCWSTR module, Target_i * target, Level lvl = defaultLevel);

			void del_module(PCWSTR module);

		private:
			virtual Module_i & get_module_(PCWSTR module) const = 0;

			virtual void add_module_(PCWSTR module, Target_i * target, Level lvl) = 0;

			virtual void del_module_(PCWSTR module) = 0;
		};


		Logger_i & get_instance();

		void init(Target_i * target, Level lvl = defaultLevel);

		void set_target(Target_i * target, PCWSTR module = defaultModule);

		void set_level(Level lvl, PCWSTR module = defaultModule);

		void set_wideness(Wideness wide, PCWSTR module = defaultModule);

		void set_color_mode(bool mode, PCWSTR module = defaultModule);

	}
}

#ifdef NDEBUG
#define LogTrace()
#define LogDebug(format, args ...)
#else
#define LogTrace()	Base::Logger::get_instance()[Logger::defaultModule].out(THIS_PLACE, Logger::LVL_TRACE, L"\n")
#define LogDebug(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(THIS_PLACE, Logger::LVL_DEBUG, format, ##args)
#endif
#define LogInfo(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(Logger::LVL_INFO, format, ##args)
#define LogReport(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(Logger::LVL_REPORT, format, ##args)
#define LogAtten(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(Logger::LVL_ATTEN, format, ##args)
#define LogWarn(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(Logger::LVL_WARN, format, ##args)
#define LogError(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(Logger::LVL_ERROR, format, ##args)
#define LogFatal(format, args ...)	Base::Logger::get_instance()[Logger::defaultModule].out(Logger::LVL_FATAL, format, ##args)

#endif
