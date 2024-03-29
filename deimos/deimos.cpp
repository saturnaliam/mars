#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <fstream>

// minhook setup
#include <MinHook.h>
#pragma comment(lib, "libMinHook.x86.lib")

// enabling ANSI escape codes
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN  0x0008

// horrifying minhook stuff
typedef BOOL(WINAPI* writeFile)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
writeFile pWriteFile = nullptr;

BOOL WINAPI detourWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
  static int currentFile = 0;

  std::string buffer;
  for (int i = 0; i < nNumberOfBytesToWrite; i++) {
    buffer = ((const char*)(lpBuffer))[i]; // fucked up hack
  }

  std::cout << "\x1b[Kwriting to save" << ++currentFile << ".dat\r";
  std::string filename = "saves\\save" + std::to_string(currentFile) + ".dat";
  std::ofstream file(filename);
  file << buffer;
  file.close();

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

/**
 * @brief Prints whether the hook is enabled / disabled bc i dont wanna retype that horrible ANSI stuff
 */
inline void disabled_enabled_printing(const std::string &message, const int &color) {
  const std::string reset_lines = "\x1b[H\n\n\n\n\n\n\x1b[K";
  const std::string text_color = "\x1b[38;5;" + std::to_string(color) + "m";
  std::cout << reset_lines << "Hook [" << text_color << message << "\x1b[38;5;252m]\n";
}

void __stdcall attachedMain(HMODULE hModule) {
  // setting up the console
  AllocConsole();
  FILE* fstdout;
  freopen_s(&fstdout, "CONOUT$", "w", stdout);

  // setting up ANSI escape codes
  HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD consoleMode;
  GetConsoleMode( handleOut , &consoleMode);
  consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;            
  SetConsoleMode( handleOut , consoleMode );

  SetConsoleTitleA("deimos");

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

  // the main loop ft. horrible ansi stuff
  bool enabled = false;  

  std::cout << "\x1b[38;5;252m";
  std::cout << R"(   _     _               
 _| |___|_|_____ ___ ___ 
| . | -_| |     | . |_ -|
|___|___|_|_|_|_|___|___|
                         )";
  std::cout << "\n[E]nable Hook // [D]isable Hook // [Q]uit\n";
  while (!GetAsyncKeyState('Q')) {
    if (GetAsyncKeyState('E') && !enabled) {
      enabled = true;
      disabled_enabled_printing("ENABLED", 78);
      MH_EnableHook(MH_ALL_HOOKS);
    } else if (GetAsyncKeyState('D') && enabled) {
      enabled = false;
      disabled_enabled_printing("DISABLED", 203);
      MH_DisableHook(MH_ALL_HOOKS);
    }
  }

  shutdown(hModule, fstdout);
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)attachedMain, hModule, 0, NULL);
  }

  return 1;
}