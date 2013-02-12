#include <libbase/logger.hpp>
#include <libbase/thread.hpp>
#include <libbase/err.hpp>
#include <libbase/str.hpp>

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

		NTSTATUS
		WINAPI
		NtQueryInformationThread(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, IN ULONG ThreadInformationLength, PULONG ReturnLength);
	}


	Thread::~Thread() noexcept
	{
		if (m_handle) {
			::CloseHandle(m_handle);
			LogNoise(L"id: %u\n", m_id);
		}

	}

	Thread::Thread(ThreadRoutine_i * routine):
		m_routine(routine),
		m_handle(::CreateThread(nullptr, 0, ThreadRoutine_i::run_thread, m_routine, CREATE_SUSPENDED, &m_id))
	{
		LogNoise(L"id: %u\n", m_id);
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
		LogNoise(L"id: %u\n", m_id);
		LogErrorIf(!::QueueUserAPC(ThreadRoutine_i::alert_thread, m_handle, (ULONG_PTR)m_routine), L"%s\n", ErrAsStr().c_str());
	}

	bool Thread::set_priority(Thread::Priority_t prio)
	{
		LogNoise(L"id: %u, prio: %s\n", m_id, as_str(prio));
		bool ret = ::SetThreadPriority(m_handle, (int)prio);
		LogErrorIf(!ret, L"%s\n", ErrAsStr().c_str());
		return ret;
	}

	bool Thread::set_io_priority(Thread::IoPriority_t prio)
	{
		LogNoise(L"id: %u, prio: %s\n", m_id, as_str(prio));
		ULONG p = (ULONG)prio;
		NTSTATUS ret = NtSetInformationThread(m_handle, ThreadIoPriority, &p, sizeof(p));
		LogErrorIf(ret, L"%s\n", NTStatusAsStr(ret).c_str());
		return ret;
	}

	size_t Thread::get_exitcode() const
	{
		LogNoise(L"id: %u\n", m_id);
		DWORD ret;
		LogErrorIf(!::GetExitCodeThread(m_handle, &ret), L"%s\n", ErrAsStr(ret).c_str());
		return ret;
	}

	Thread::Priority_t Thread::get_priority() const
	{
		LogNoise(L"id: %u\n", m_id);
		return (Thread::Priority_t)::GetThreadPriority(m_handle);
	}

	Thread::IoPriority_t Thread::get_io_priority() const
	{
		LogNoise(L"id: %u\n", m_id);
		ULONG p = 0;
		NTSTATUS ret = NtQueryInformationThread(m_handle, ThreadIoPriority, &p, sizeof(p), nullptr);
		LogErrorIf(ret, L"%s\n", NTStatusAsStr(ret).c_str());
		return (Thread::IoPriority_t)p;
	}

	bool Thread::suspend() const
	{
		LogNoise(L"id: %u\n", m_id);
		bool ret = ::SuspendThread(m_handle) != (DWORD)-1;
		LogErrorIf(!ret, L"%s\n", ErrAsStr().c_str());
		return ret;
	}

	bool Thread::resume() const
	{
		LogNoise(L"id: %u\n", m_id);
		bool ret = ::ResumeThread(m_handle) != (DWORD)-1;
		LogErrorIf(!ret, L"%s\n", ErrAsStr().c_str());
		return ret;
	}

	WaitResult_t Thread::wait(Timeout_t timeout) const
	{
		LogNoise(L"id: %u\n", m_id);
		return (WaitResult_t)::WaitForSingleObjectEx(m_handle, timeout, true);
	}

	PCWSTR as_str(Thread::Priority_t prio)
	{
		switch (prio) {
			case Thread::Priority_t::IDLE:
				return L"Idle";
			case Thread::Priority_t::LOWEST:
				return L"Lowest";
			case Thread::Priority_t::BELOW_NORMAL:
				return L"Below normal";
			case Thread::Priority_t::NORMAL:
				return L"Normal";
			case Thread::Priority_t::ABOVE_NORMAL:
				return L"Above normal";
			case Thread::Priority_t::HIGHEST:
				return L"Highest";
			case Thread::Priority_t::TIME_CRITICAL:
				return L"Time critical";
		}
		return L"Unknown";
	}

	PCWSTR as_str(Thread::IoPriority_t prio)
	{
		switch (prio) {
			case Thread::IoPriority_t::VERY_LOW:
				return L"Very low";
			case Thread::IoPriority_t::LOW:
				return L"Low";
			case Thread::IoPriority_t::NORMAL:
				return L"Normal";
			case Thread::IoPriority_t::HIGH:
				return L"High";
			case Thread::IoPriority_t::CRITICAL:
				return L"Critical";
		}
		return L"Unknown";
	}
}
