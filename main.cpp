#include <stdio.h>
#include <Windows.h>

SYSTEM_INFO sysInfo;

DWORD
CreateThreadAffinityMask(DWORD ProcessorNumber) {
  return (1 << ProcessorNumber);
}

DWORD
ThreadProc(PVOID ThreadParameter) {

  DWORD CoreCount = PtrToUlong(ThreadParameter);

  printf("Thread 0x%08X running on core %d.\n",
         GetCurrentThreadId(),
         CoreCount);

  return 1;
}


int main() {
  printf("Multicore Thread Test version 1.0\n");

  PHANDLE hThreads;
  SYSTEM_INFO sysInfo;
  DWORD CoreCount;

  //
  // Get the core information
  //
  GetSystemInfo(&sysInfo);  
  CoreCount = sysInfo.dwNumberOfProcessors;

  //
  // Allocate memory for the thread handles
  //
  hThreads = (PHANDLE)HeapAlloc(GetProcessHeap(),
                                0,
                                sizeof(HANDLE) * CoreCount);

  if (!hThreads) {
    printf("Failed to allocate memory for thread handles.\n");
  }

  //
  // Create the threads
  //
  
  for (int i = 0; i < CoreCount; i++) {

    hThreads[i] = CreateThread(NULL,
                               0,
                               &ThreadProc,
                               (LPVOID)i,
                               CREATE_SUSPENDED,
                               NULL);

    if (hThreads[i] == INVALID_HANDLE_VALUE) {
      printf("Failed to create thread. Error == %d\n",
             GetLastError());
      return 0;
    }

    SetThreadAffinityMask(hThreads[i],
                          CreateThreadAffinityMask(i));

    ResumeThread(hThreads[i]);

    printf("Created thread with handle %d.\n",
           hThreads[i]);
  }

  WaitForMultipleObjects(CoreCount,
                         hThreads,
                         TRUE,
                         INFINITE);

  return 1;
}
