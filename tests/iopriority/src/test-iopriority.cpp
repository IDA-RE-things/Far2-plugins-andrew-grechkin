//============================================================================
// Name        : test-iopriority.cpp
// Author      : Andrew Grechkin
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <windows.h>
//#include <ddk/ntddk.h>
//#include <ddk/ntifs.h>

//#include <iostream>
using namespace std;

namespace  ThreadPriorityIo  {
	enum {
		very_low,
		low,
		normal,
		high,
		critical,
	};
}

typedef LONG NTSTATUS;

typedef enum _THREADINFOCLASS {
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
} THREADINFOCLASS;


extern "C" {
	NTSYSCALLAPI
	NTSTATUS
	NTAPI
	NtSetInformationThread(
	  IN HANDLE ThreadHandle,
	  IN THREADINFOCLASS ThreadInformationClass,
	  IN PVOID ThreadInformation,
	  IN ULONG ThreadInformationLength);

}

int main() {
	HANDLE thread = ::GetCurrentThread();
	ULONG prio = ThreadPriorityIo::very_low;
	Sleep(10000);
	NtSetInformationThread(thread, ThreadIoPriority, &prio, sizeof(prio));
	Sleep(10000);
	++prio;
	NtSetInformationThread(thread, ThreadIoPriority, &prio, sizeof(prio));
	Sleep(10000);
	++prio;
	NtSetInformationThread(thread, ThreadIoPriority, &prio, sizeof(prio));
	Sleep(10000);
	return 0;
}
