#include <libbase/logger.hpp>

#include <libbase/va_list.hpp>
#include <libbase/shared_ptr.hpp>
#include <libbase/str.hpp>

namespace Base {
	namespace Logger {

		PCWSTR LogLevelNames[LVL_END__] = {L"TRACE ", L"DEBUG ", L"INFO  ", L"REPRT ", L"ATTEN ", L"WARN  ", L"ERROR ", L"FATAL ", };

		struct FmtString {
			PCWSTR const place;
			PCWSTR const additional;
		};

		FmtString fmtStrings[WIDE_END__] = {{L"%S: %d [%S] ", L"%s[%s] (%u) "}, {L"%S: %d [%S] ", L"%s[%s] "}, {L"%S: %d [%S] ", L"%s"}, {L"%S: %d [%S] ", L""}, };

		///==================================================================================== Target_i
		Target_i::~Target_i() {
		}

		///==================================================================================== Logger_i
		Logger_i::~Logger_i() {
		}

		///====================================================================================== Logger
		struct Logger_impl: public Logger_i {
			Logger_impl();

			virtual ~Logger_impl();

			virtual Level get_level() const;

			virtual Wideness get_wideness() const;

			virtual void set_level(Level lvl);

			virtual void set_wideness(Wideness mode);

			virtual void set_color_mode(bool mode);

			virtual bool is_color_mode() const;

			virtual void set_target(Target_i * target);

			virtual void out(PCSTR file, int line, PCSTR func, Level lvl, PCWSTR module, PCWSTR format, ...) const;

			virtual void out(Level lvl, PCWSTR module, PCWSTR format, ...) const;

		private:
			void out_args(Level lvl, const ustring & prefix, PCWSTR format, va_list args) const;

			shared_ptr<Target_i> m_target;

			Level m_lvl;
			Wideness m_wide;

			struct {
				uint8_t m_color :1;
			};
		};

		Logger_impl::~Logger_impl() {
		}

		Logger_impl::Logger_impl() :
			m_lvl(defaultLevel), m_wide(defaultWideness), m_color(0) {
		}

		Level Logger_impl::get_level() const {
			return m_lvl;
		}

		Wideness Logger_impl::get_wideness() const {
			return m_wide;
		}

		void Logger_impl::set_level(Level lvl) {
			m_lvl = lvl;
		}

		void Logger_impl::set_wideness(Wideness wide) {
			m_wide = wide;
		}

		void Logger_impl::set_color_mode(bool mode) {
			m_color = mode;
		}

		bool Logger_impl::is_color_mode() const {
			return m_color;
		}

		void Logger_impl::set_target(Target_i * target) {
			m_target.reset(target);
		}

		void Logger_impl::out(PCSTR file, int line, PCSTR func, Level lvl, PCWSTR module, PCWSTR format, ...) const {
			if (lvl >= m_lvl) {
				va_list args;
				va_start(args, format);
				ustring tmp = as_str(fmtStrings[m_wide].additional, LogLevelNames[lvl], module, ::GetCurrentThreadId());
				tmp += as_str(fmtStrings[m_wide].place, file, line, func);
				;
				out_args(lvl, tmp, format, args);
				va_end(args);
			}
		}

		void Logger_impl::out(Level lvl, PCWSTR module, PCWSTR format, ...) const {
			if (lvl >= m_lvl) {
				va_list args;
				va_start(args, format);
				ustring tmp = as_str(fmtStrings[m_wide].additional, LogLevelNames[lvl], module, ::GetCurrentThreadId());
				out_args(lvl, tmp, format, args);
				va_end(args);
			}
		}

		void Logger_impl::out_args(Level lvl, const ustring & prefix, PCWSTR format, va_list args) const {
			ustring tmp(prefix);
			tmp += as_str(format, args);
			m_target->out(this, lvl, tmp.c_str(), tmp.size());
		}

		Logger_i & get_instance() {
			static Logger_impl ret;
			return ret;
		}

		void init(Target_i * target, Level lvl) {
			get_instance().set_target(target);
			get_instance().set_level(lvl);
		}

		void set_level(Level lvl) {
			get_instance().set_level(lvl);
		}

		void set_wideness(Wideness mode) {
			get_instance().set_wideness(mode);
		}

		void set_color_mode(bool mode) {
			get_instance().set_color_mode(mode);
		}

	}
}
