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
		NtSetInformationThread(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, IN PVOID ThreadInformation, IN ULONG ThreadInformationLength);
	}


	Thread::~Thread() noexcept
	{
		::CloseHandle(m_handle);
	}

	Thread::Thread(ThreadRoutine_i * routine):
		m_routine(routine),
		m_handle(::CreateThread(nullptr, 0, ThreadRoutine_i::run_thread, m_routine, CREATE_SUSPENDED, &m_id))
	{
	}

	Thread::Thread(Thread && right):
		m_routine(nullptr),
		m_handle(nullptr),
		m_id(0)
	{
		swap(right);
	}

	Thread & Thread::operator = (Thread && right)
	{
		if (this != &right)
			Thread(std::move(right)).swap(*this);
		return *this;
	}

	void Thread::swap(Thread & right) noexcept
	{
		using std::swap;
		swap(m_handle, right.m_handle);
		swap(m_routine, right.m_routine);
		swap(m_id, right.m_id);
	}

	void Thread::alert()
	{
		::QueueUserAPC(ThreadRoutine_i::alert_thread, m_handle, (ULONG_PTR)m_routine);
	}

	bool Thread::set_priority(Thread::Priority_t prio)
	{
		return ::SetThreadPriority(m_handle, (int)prio);
	}

	bool Thread::set_io_priority(Thread::IoPriority_t prio)
	{
		ULONG p = (ULONG)prio;
		return NtSetInformationThread(m_handle, ThreadIoPriority, &p, sizeof(prio));
	}

	size_t Thread::get_exitcode() const
	{
		DWORD ret;
		::GetExitCodeThread(m_handle, &ret);
		return ret;
	}

	Thread::Priority_t Thread::get_priority() const
	{
		return (Thread::Priority_t)::GetThreadPriority(m_handle);
	}

	bool Thread::suspend() const
	{
		return ::SuspendThread(m_handle) < (DWORD)-1;
	}

	bool Thread::resume() const
	{
		return ::ResumeThread(m_handle) < (DWORD)-1;
	}

	bool Thread::wait(Thread::timeout_t timeout) const
	{
		return ::WaitForSingleObjectEx(m_handle, timeout, TRUE) == WAIT_OBJECT_0;
	}

}
