#ifndef _LIBBASE_LOGGER_HPP_
#define _LIBBASE_LOGGER_HPP_

#include <libbase/std.hpp>


namespace Base {

	namespace Logger {

		struct Logger_i;
		struct Module_i;
		struct Target_i;

		enum class Level: ssize_t {
			Trace,
			Debug,
			Info,
			Report,
			Atten,
			Warn,
			Error,
			Fatal,
		};

		enum class Wideness: ssize_t {
			Full,
			Module,
			Medium,
			Short,
		};

		Level get_default_level();

		Wideness get_default_wideness();

		Module_i * get_default_module();


		///================================================================================ Logger_i
		struct Logger_i {
			Module_i * register_module(PCWSTR name, Target_i * target, Level lvl = get_default_level());

			void free_module(Module_i * module);

			virtual ~Logger_i();

		private:
//			virtual Module_i * get_module_(PCWSTR name) const = 0;

			virtual Module_i * register_module_(PCWSTR name, Target_i * target, Level lvl) = 0;

			virtual void free_module_(Module_i * module) = 0;
		};


#ifdef NO_LOGGER

		inline void set_target(Target_i * /*target*/, Module_i * /*module*/ = nullptr) {
		}

		inline void set_level(Level /*lvl*/, Module_i * /*module*/ = nullptr) {
		}

		inline void set_wideness(Wideness /*wide*/, Module_i * /*module*/ = nullptr) {
		}

		inline void set_color_mode(bool /*mode*/, Module_i * /*module*/ = nullptr) {
		}

#else

		Logger_i & get_instance();

		void set_target(Target_i * target, Module_i * module = get_default_module());

		void set_level(Level lvl, Module_i * module = get_default_module());

		void set_wideness(Wideness wide, Module_i * module = get_default_module());

		void set_color_mode(bool mode, Module_i * module = get_default_module());

#endif


		///================================================================================ Module_i
		struct Module_i {
			virtual ~Module_i();

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

			virtual ssize_t get_index() const = 0;
		};


		///================================================================================ Target_i
		struct Target_i {
			virtual ~Target_i();

			virtual void out(const Module_i * module, Level lvl, PCWSTR str, size_t size) const = 0;

			virtual void out(PCWSTR str, size_t size) const = 0;
		};


#ifdef NO_LOGGER

		inline Target_i * get_TargetToNull() {
			return nullptr;
		}

		inline Target_i * get_TargetToConsole() {
			return nullptr;
		}

		inline Target_i * get_TargetToFile(PCWSTR /*path*/) {
			return nullptr;
		}

		inline Target_i * get_TargetToSys(PCWSTR /*name*/, PCWSTR /*path*/ = nullptr) {
			return nullptr;
		}

#else

		Target_i * get_TargetToNull();

		Target_i * get_TargetToConsole();

		Target_i * get_TargetToFile(PCWSTR path);

		Target_i * get_TargetToSys(PCWSTR name, PCWSTR path = nullptr);

#endif


	}

}

#ifdef NO_LOGGER
#	define LogTrace()
#	define LogTraceIf(condition) (condition)
#	define LogNoise(format, args ...)
#	define LogDebug(format, args ...)
#	define LogDebugIf(condition, format, args ...) (condition)
#	define LogInfo(format, args ...)
#	define LogReport(format, args ...)
#	define LogAtten(format, args ...)
#	define LogWarn(format, args ...)
#	define LogWarnIf(condition, format, args ...) (condition)
#	define LogError(format, args ...)
#	define LogErrorIf(condition, format, args ...) (condition)
#	define LogFatal(format, args ...)
#else
#	ifdef NO_TRACE
#		define LogTrace()
#		define LogTraceIf(condition) (condition)
#		define LogNoise(format, args ...)
#		define LogDebug(format, args ...)
#		define LogDebugIf(condition, format, args ...) (condition)
#	else
#       define LogTrace()                   Base::Logger::get_default_module()->out(THIS_PLACE, Base::Logger::Level::Trace, L"\n")
#       define LogTraceIf(condition)        if (condition) Base::Logger::get_default_module()->out(THIS_PLACE, Base::Logger::Level::Trace, L"\n")
#       define LogNoise(format, args ...)	Base::Logger::get_default_module()->out(THIS_PLACE, Base::Logger::Level::Trace, format, ##args)
#       define LogDebug(format, args ...)	Base::Logger::get_default_module()->out(THIS_PLACE, Base::Logger::Level::Debug, format, ##args)
#       define LogDebugIf(condition, format, args ...)	if (condition) Base::Logger::get_default_module()->out(THIS_PLACE, Base::Logger::Level::Debug, format, ##args)
#	endif

#	define LogInfo(format, args ...)	Base::Logger::get_default_module()->out(Base::Logger::Level::Info, format, ##args)
#	define LogReport(format, args ...)	Base::Logger::get_default_module()->out(Base::Logger::Level::Report, format, ##args)
#	define LogAtten(format, args ...)	Base::Logger::get_default_module()->out(Base::Logger::Level::Atten, format, ##args)
#	define LogWarn(format, args ...)	Base::Logger::get_default_module()->out(Base::Logger::Level::Warn, format, ##args)
#   define LogWarnIf(condition, format, args ...) if (condition) Base::Logger::get_default_module()->out(Base::Logger::Level::Warn, format, ##args)
#   define LogError(format, args ...)	Base::Logger::get_default_module()->out(Base::Logger::Level::Error, format, ##args)
#   define LogErrorIf(condition, format, args ...) if (condition) Base::Logger::get_default_module()->out(Base::Logger::Level::Error, format, ##args)
#	define LogFatal(format, args ...)	Base::Logger::get_default_module()->out(Base::Logger::Level::Fatal, format, ##args)
#endif

#endif
