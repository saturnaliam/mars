#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <string>
#include <iostream>

// minhook setup
#include <MinHook.h>
#pragma comment(lib, "libMinHook.x86.lib")

// horrifying minhook stuff
typedef BOOL(WINAPI* writeFile)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
writeFile pWriteFile = nullptr;

BOOL WINAPI detourWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
  std::cout << "file written\n";
  return pWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

/**
 * @brief Shuts the progam down.
 * @param hModule The thread's HMODULE.
 * @param fstdout The stdout handle.
 * @param exitCode Exit code, default 0.
 */
void __stdcall shutdown(HMODULE hModule, FILE* fstdout, DWORD exitCode = 0) {
  if (fstdout != nullptr) fclose(fstdout);
  MH_Uninitialize();
  FreeConsole();
  FreeLibraryAndExitThread(hModule, exitCode);
}

/**
 * @brief Panics and ends the program.
 * @param message The message.
 * @param hModule HMODULE of the thread.
 * @param fstdout The stdout handle, default nullptr.
 */
void __stdcall panic(std::string message, HMODULE hModule, FILE* fstdout = nullptr) {
  MessageBoxA(0, message.c_str(), "deimos panicked!", MB_OK | MB_ICONERROR);
  shutdown(hModule, fstdout, 1);

  exit(1);
}

void __stdcall attachedMain(HMODULE hModule) {
  // setting up the console
  AllocConsole();
  FILE* fstdout;
  freopen_s(&fstdout, "CONOUT$", "w", stdout);

  if (fstdout == nullptr) {
    panic("failed to open stdout!", hModule);
  }

  // setting up minhook
  if (MH_Initialize() != MH_OK) {
    panic("failed to initialize minhook!", hModule, fstdout);
  }

  if (MH_CreateHookApi(L"Kernel32", "WriteFile", &detourWriteFile, reinterpret_cast<void**>(&pWriteFile)) != MH_OK) {
    panic("hook failed!", hModule, fstdout);
  }

  MH_EnableHook(MH_ALL_HOOKS);
  while (!GetAsyncKeyState('Q')) {}

  shutdown(hModule, fstdout);
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)attachedMain, hModule, 0, NULL);
  }

  return 1;
}