#ifndef _LIBBASE_THREADROUTINE_I_HPP_
#define _LIBBASE_THREADROUTINE_I_HPP_

#include <libbase/std.hpp>
#include <libbase/messaging.hpp>

namespace Base {

	struct ThreadRoutine_i {
		static DWORD WINAPI run_thread(void * routine);

		static VOID WINAPI alert_thread(ULONG_PTR routine);

	public:
		virtual ~ThreadRoutine_i();

		virtual void alert(void * data);

		virtual size_t run(void * data);

		virtual void post_message(const Message & message) const;
	};

}

#endif
