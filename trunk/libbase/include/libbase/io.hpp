#ifndef _LIBBASE_IO_HPP_
#define _LIBBASE_IO_HPP_

#include <libbase/std.hpp>

namespace Base {

	typedef LONG NTSTATUS;

	namespace ThreadIo {
		enum Priority {
			very_low,
			low,
			normal,
			high,
			critical,
		};
	}

	NTSTATUS set_thread_io_priority(HANDLE thread, ThreadIo::Priority priority);
}

#endif
