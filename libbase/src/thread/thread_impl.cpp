#include <libbase/thread.hpp>

namespace Base {
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


	Thread::~Thread() {
		::CloseHandle(m_handle);
		delete m_routine;
	}

	Thread::Thread(ThreadRoutine_i * routine, void * data, size_t stack_size):
		m_routine(routine),
		m_handle(::CreateThread(nullptr, stack_size, ThreadParameters::run_thread_with_param, new ThreadParameters(m_routine, data), CREATE_SUSPENDED, &m_id)) {
	}

	Thread::Thread(Thread && rhs):
		m_routine(nullptr),
		m_handle(nullptr),
		m_id(0)
	{
		this->swap(rhs);
	}

	Thread & Thread::operator = (Thread && rhs) {
		if (this != &rhs)
			swap(rhs);
		return *this;
	}

	void Thread::swap(Thread & rhs) {
		using std::swap;
		swap(m_handle, rhs.m_handle);
		swap(m_routine, rhs.m_routine);
		swap(m_id, rhs.m_id);
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
		return m_routine;
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

	void Thread::alert(void * data) {
		::QueueUserAPC(ThreadParameters::alert_thread_with_param, m_handle, (ULONG_PTR)new ThreadParameters(m_routine, data));
	}

}
