#include <libbase/logger.hpp>

#include <libbase/lock.hpp>
#include <libbase/console.hpp>

namespace Base {
	namespace Logger {

		WORD LogLevelColors[LVL_END__] = {0x8, 0x6, 0, 0x3, 0xA, 0xE, 0xC, 0x4, };

		struct LogToConsole: public Target_i, private Uncopyable {
			virtual ~LogToConsole();

			virtual void out(const Logger_i * lgr, Level lvl, PCWSTR str, size_t size) const;

			LogToConsole();

		private:
			Base::SyncUnit_i * m_sync;
		};

		LogToConsole::~LogToConsole() {
			delete m_sync;
		}

		LogToConsole::LogToConsole() :
			m_sync(Base::get_LockCritSection()) {
		}

		void LogToConsole::out(const Logger_i * lgr, Level lvl, PCWSTR str, size_t size) const {
			auto lk(m_sync->get_lock());
			if (lgr->is_color_mode()) {
				ConsoleColor color(LogLevelColors[lvl]);
				consoleout(str, size);
			} else {
				consoleout(str, size);
			}
		}

		Target_i * get_TargetToConsole() {
			return new LogToConsole();
		}

	}
}
