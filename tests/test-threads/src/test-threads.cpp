#include <libbase/std.hpp>
#include <libbase/logger.hpp>
#include <libbase/messaging.hpp>
#include <libbase/thread.hpp>

#include <stdio.h>
#include <functional>
#include <memory>
#include <vector>

struct Routine: public Base::ThreadRoutine_i
{
	size_t run(void *) override
	{
		Sleep(15000);
		return 42;
	}

private:
	static Base::Queue m_queue;
};

int main()
{
	Base::Logger::set_target(Base::Logger::get_TargetToConsole());
	Base::Logger::set_level(Base::Logger::Level::Trace);
	Base::Logger::set_wideness(Base::Logger::Wideness::Full);
	Base::Logger::set_color_mode(true);

	Routine routine;

	std::vector<Base::Thread> threads;
	threads.emplace_back(Base::Thread(&routine));
	threads.emplace_back(Base::Thread(&routine));

	Sleep(5000);
	threads[0].set_io_priority(Base::Thread::IoPriority_t::LOW);
	threads[1].set_io_priority(Base::Thread::IoPriority_t::HIGH);

	Sleep(5000);
	threads[0].set_priority(Base::Thread::Priority_t::TIME_CRITICAL);
	threads[1].set_priority(Base::Thread::Priority_t::ABOVE_NORMAL);

	threads[0].resume();
	threads[1].resume();
	threads[0].wait();
	threads[1].wait();

	LogInfo(L"threads[0] exited: %d\n", threads[0].get_exitcode());
	LogInfo(L"threads[1] exited: %d\n", threads[1].get_exitcode());
	return 0;
}
