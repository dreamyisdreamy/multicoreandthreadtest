#include <stdio.h>
#include <Windows.h>

SYSTEM_INFO sysInfo;

DWORD
CreateThreadAffinityMask(DWORD ProcessorNumber) {
  return (1 << ProcessorNumber);
}

ULONGLONG fibonacci(unsigned int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

DWORD
ThreadProc(PVOID ThreadParameter) {

  DWORD CoreNumber = PtrToUlong(ThreadParameter);
  ULONGLONG StartCount = 0;
  ULONGLONG StopCount = 0;
  
  printf("Thread 0x%08X running on core %d.\n",
         GetCurrentThreadId(),
         CoreNumber);

  //
  // Run the test load
  //
  StartCount = GetTickCount64();

  fibonacci(50);

  StopCount = GetTickCount64();

  printf("Core %d test ran for %llu ticks.\n",
         CoreNumber,
         StopCount - StartCount);

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
