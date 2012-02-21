#ifndef _WIN_DEF_IO_HPP
#define _WIN_DEF_IO_HPP

#include "std.h"

namespace windef {
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
