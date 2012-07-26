#include <libbase/thread.hpp>

enum THREADINFOCLASS {
	ThreadBasicInformation,
	ThreadTimes,
	ThreadPriority,
	ThreadBasePriority,
	ThreadAffinityMask,
	ThreadImpersonationToken,
	ThreadDescriptorTableEntry,
	ThreadEnableAlignmentFaultFixup,
	ThreadEventPair_Reusable,
	ThreadQuerySetWin32StartAddress,
	ThreadZeroTlsCell,
	ThreadPerformanceCount,
	ThreadAmILastThread,
	ThreadIdealProcessor,
	ThreadPriorityBoost,
	ThreadSetTlsArrayAddress,
	ThreadIsIoPending,
	ThreadHideFromDebugger,
	ThreadBreakOnTermination,
	ThreadSwitchLegacyState,
	ThreadIsTerminated,
	ThreadLastSystemCall,
	ThreadIoPriority,
	ThreadCycleTime,
	ThreadPagePriority,
	ThreadActualBasePriority,
	ThreadTebInformation,
	ThreadCSwitchMon,
	ThreadCSwitchPmu,
	ThreadWow64Context,
	ThreadGroupInformation,
	ThreadUmsInformation,
	ThreadCounterProfiling,
	ThreadIdealProcessorEx,
	MaxThreadInfoClass
};

typedef LONG NTSTATUS;

extern "C" {
	NTSYSCALLAPI
	NTSTATUS
	NTAPI
	NtSetInformationThread( IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, IN PVOID ThreadInformation, IN ULONG ThreadInformationLength);
}

namespace Base {

	ThreadRoutine_i::~ThreadRoutine_i() {
	}

	size_t ThreadRoutine_i::run()  {
		LogTrace();
		return 0;
	}


	Thread::~Thread() {
//		::CloseHandle(m_handle);
	}

	Thread::Thread(ThreadRoutine_i * routine, size_t stack_size):
		m_routine(routine),
		m_handle(::CreateThread(nullptr, stack_size, member_thunk<ThreadRoutine_i, &ThreadRoutine_i::run>, routine, CREATE_SUSPENDED, &m_id)) {
		LogDebug(L"m_handle: %p\n", m_handle.data());
	}

	bool Thread::set_priority(Thread::priority_t prio) {
		return ::SetThreadPriority(m_handle, (int)prio);
	}

	bool Thread::set_io_priority(Thread::io_priority_t prio) {
		ULONG p = (ULONG)prio;
		return NtSetInformationThread(m_handle, ThreadIoPriority, &p, sizeof(prio));
	}

	size_t Thread::get_exitcode() const {
		DWORD ret;
		::GetExitCodeThread(m_handle, &ret);
		return ret;
	}

	size_t Thread::get_id() const {
		return m_id;
	}

	Thread::handle_t Thread::get_handle() const {
		return m_handle;
	}

	Thread::priority_t Thread::get_priority() const {
		return (Thread::priority_t)::GetThreadPriority(m_handle);
	}

	ThreadRoutine_i * Thread::get_routine() const {
		return m_routine.get();
	}

	bool Thread::suspend() const {
		return ::SuspendThread(m_handle) < (DWORD)-1;
	}

	bool Thread::resume() const {
		return ::ResumeThread(m_handle) < (DWORD)-1;
	}

	bool Thread::wait(Thread::timeout_t timeout) const {
		return ::WaitForSingleObjectEx(m_handle, timeout, FALSE) == WAIT_OBJECT_0;
	}


	void ThreadsHolder::push_back(ThreadRoutine_i * routine) {
		m_threads.emplace_back(routine);
//			m_handles.push_back(thread.get_handle());
	}


}
