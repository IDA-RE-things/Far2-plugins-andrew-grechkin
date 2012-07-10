#include <libbase/logger.hpp>

#include <libbase/va_list.hpp>
#include <libbase/lock.hpp>
#include <libbase/shared_ptr.hpp>
#include <libbase/str.hpp>

#include <vector>

namespace Base {
	namespace Logger {

		PCWSTR const LogLevelNames[LVL_FATAL + 1] = {
			L"TRACE ",
			L"DEBUG ",
			L"INFO  ",
			L"REPRT ",
			L"ATTEN ",
			L"WARN  ",
			L"ERROR ",
			L"FATAL ",
		};

		struct FmtString {
			PCWSTR const place;
			PCWSTR const additional;
		};

		const FmtString fmtStrings[WIDE_SHORT + 1] = {
			{L"%S: %d [%S] ", L"%s{%s}:%u "},
			{L"%S: %d [%S] ", L"%s{%s} "},
			{L"%S: %d [%S] ", L"%s"},
			{L"%S: %d [%S] ", L""},
		};

		///================================================================================ Target_i
		Target_i::~Target_i() {
		}

		struct LogToNull: public Target_i {
			virtual ~LogToNull();

			virtual void out(const Module_i * lgr, Level lvl, PCWSTR str, size_t size) const;
		};

		LogToNull::~LogToNull() {
		}

		void LogToNull::out(const Module_i * /*lgr*/, Level /*lvl*/, PCWSTR /*str*/, size_t /*size*/) const {
		}

		///================================================================================ Module_i
		Module_i::~Module_i() {
		}

		///============================================================================= Module_impl
		struct Module_impl: public Module_i, private Uncopyable {
			Module_impl(PCWSTR name, Target_i * tgt, Level lvl, ssize_t index);

			virtual ~Module_impl();

			virtual PCWSTR get_name() const;

			virtual Level get_level() const;

			virtual Wideness get_wideness() const;

			virtual void set_level(Level lvl);

			virtual void set_wideness(Wideness mode);

			virtual void set_color_mode(bool mode);

			virtual bool is_color_mode() const;

			virtual void set_target(Target_i * target);

			virtual void out(PCSTR file, int line, PCSTR func, Level lvl, PCWSTR format, ...) const;

			virtual void out(Level lvl, PCWSTR format, ...) const;

			virtual ssize_t get_index() const;

		private:
			void out_args(Level lvl, const ustring & prefix, PCWSTR format, va_list args) const;

			auto_array<WCHAR> m_name;
			shared_ptr<Target_i> m_target;

			ssize_t m_index;

			Level m_lvl;
			Wideness m_wide;

			struct {
				uint8_t m_color :1;
			};

			friend class Logger_impl;
		};

		Module_impl::Module_impl(PCWSTR name, Target_i * tgt, Level lvl, ssize_t index):
			m_name(get_str_len(name) + 1, name),
			m_target(tgt),
			m_index(index),
			m_lvl(lvl),
			m_wide(defaultWideness),
			m_color(0) {
		}

		Module_impl::~Module_impl() {
		}

		PCWSTR Module_impl::get_name() const {
			return m_name.data();
		}

		Level Module_impl::get_level() const {
			return m_lvl;
		}

		Wideness Module_impl::get_wideness() const {
			return m_wide;
		}

		void Module_impl::set_level(Level lvl) {
			m_lvl = lvl;
		}

		void Module_impl::set_wideness(Wideness wide) {
			m_wide = wide;
		}

		void Module_impl::set_color_mode(bool mode) {
			m_color = mode;
		}

		bool Module_impl::is_color_mode() const {
			return m_color;
		}

		void Module_impl::set_target(Target_i * target) {
			m_target.reset(target);
		}

		void Module_impl::out(PCSTR file, int line, PCSTR func, Level lvl, PCWSTR format, ...) const {
			if (lvl >= m_lvl) {
				va_list args;
				va_start(args, format);
				ustring tmp = as_str(fmtStrings[m_wide].additional, LogLevelNames[lvl], m_name.data(), ::GetCurrentThreadId());
				tmp += as_str(fmtStrings[m_wide].place, file, line, func);
				;
				out_args(lvl, tmp, format, args);
				va_end(args);
			}
		}

		void Module_impl::out(Level lvl, PCWSTR format, ...) const {
			if (lvl >= m_lvl) {
				va_list args;
				va_start(args, format);
				ustring tmp = as_str(fmtStrings[m_wide].additional, LogLevelNames[lvl], m_name.data(), ::GetCurrentThreadId());
				out_args(lvl, tmp, format, args);
				va_end(args);
			}
		}

		void Module_impl::out_args(Level lvl, const ustring & prefix, PCWSTR format, va_list args) const {
			ustring tmp(prefix);
			tmp += as_str(format, args);
			m_target->out(this, lvl, tmp.c_str(), tmp.size());
		}

		ssize_t Module_impl::get_index() const {
			return m_index;
		}

		struct pModule_pModule_less: public std::binary_function<const Module_i *, const Module_i *, bool> {
			bool operator () (const Module_i * lhs, const Module_i * rhs) {
				return compare_str(lhs->get_name(), rhs->get_name()) < 0;
			}
		};

		struct pModule_PCWSTR_less: public std::binary_function<const Module_i *, PCWSTR, bool> {
			bool operator () (const Module_i * lhs, PCWSTR rhs) {
				return compare_str(lhs->get_name(), rhs) < 0;
			}
		};

		struct pModule_pModule_equal: public std::binary_function<const Module_i *, const Module_i *, bool> {
			bool operator () (const Module_i * lhs, const Module_i * rhs) {
				return compare_str(lhs->get_name(), rhs->get_name()) == 0;
			}
		};

		struct pModule_PCWSTR_equal: public std::binary_function<const Module_i *, PCWSTR, bool> {
			bool operator () (const Module_i * lhs, PCWSTR rhs) {
				return compare_str(lhs->get_name(), rhs) == 0;
			}
		};

		Module_i * defaultModule = nullptr;

		PCWSTR const defaultModuleName = L"default";


		///================================================================================ Logger_i
//		Module_i * Logger_i::operator [](PCWSTR name) const {
//			return get_module_(name);
//		}

		Module_i * Logger_i::register_module(PCWSTR name, Target_i * target, Level lvl) {
			return register_module_(name, target, lvl);
		}

		void Logger_i::free_module(Module_i * module) {
			free_module_(module);
		}

		Logger_i::~Logger_i() {
		}

		///============================================================================= Logger_impl
		struct Logger_impl: public Logger_i {
			Logger_impl();

			virtual ~Logger_impl();

//			virtual Module_i * get_module_(PCWSTR name) const;

			virtual Module_i * register_module_(PCWSTR name, Target_i * target, Level lvl);

			virtual void free_module_(Module_i * module);

		private:
			std::vector<Module_i*> m_modules;
			Lock::SyncUnit_i * m_sync;
		};

		Logger_impl::Logger_impl():
			m_sync(Lock::get_ReadWrite()) {
			defaultModule = register_module_(defaultModuleName, new LogToNull, LVL_FATAL);
		}

		Logger_impl::~Logger_impl() {
			{
//				auto lk(m_sync->get_lock());
				while (!m_modules.empty()) {
					free_module_(m_modules.back());
					m_modules.pop_back();
				}
			}
			delete m_sync;
		}

//		Module_i & Logger_impl::get_module_(PCWSTR name) const {
//			auto lk(m_sync->get_lock_read());
//			return *(m_modules[module.index].iface);
//		}

		Module_i * Logger_impl::register_module_(PCWSTR name, Target_i * target, Level lvl) {
			auto lk(m_sync->get_lock());
			m_modules.push_back(new Module_impl(name, target, lvl, m_modules.size()));
			return m_modules.back();
		}

		void Logger_impl::free_module_(Module_i * module) {
			auto lk(m_sync->get_lock());
			if (module) {
				ssize_t index = module->get_index();
				delete m_modules[index];
				m_modules[index] = nullptr;
			}
		}


		Module_i * get_default_module() {
			get_instance();
			return defaultModule;
		}

		Logger_i & get_instance() {
			static Logger_impl ret;
			return ret;
		}

		void set_target(Target_i * target, Module_i * module) {
			module->set_target(target);
		}

		void set_level(Level lvl, Module_i * module) {
			module->set_level(lvl);
		}

		void set_wideness(Wideness mode, Module_i * module) {
			module->set_wideness(mode);
		}

		void set_color_mode(bool mode, Module_i * module) {
			module->set_color_mode(mode);
		}

	}
}
