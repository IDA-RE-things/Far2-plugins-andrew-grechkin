#include <libbase/logger.hpp>


namespace Base {
	namespace Logger {

		struct LogToNull: public Target_i {
			virtual ~LogToNull();

			virtual void out(const Module_i * lgr, Level lvl, PCWSTR str, size_t size) const;

			virtual void out(PCWSTR str, size_t size) const;
		};


		LogToNull::~LogToNull() {
		}

		void LogToNull::out(const Module_i * /*lgr*/, Level /*lvl*/, PCWSTR /*str*/, size_t /*size*/) const {
		}

		void LogToNull::out(PCWSTR /*str*/, size_t /*size*/) const {
		}


		Target_i * get_TargetToNull() {
			return new LogToNull();
		}

	}
}
