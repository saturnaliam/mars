#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <cstdint>
#include <vector>

// enabling ANSI escape codes
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define DISABLE_NEWLINE_AUTO_RETURN  0x0008

typedef std::int32_t s32;

struct GameValues {
  volatile s32* coins;
  volatile s32* chocolate;
  volatile s32* fertilizer;
  volatile s32* bugSpray;
};

/**
 * @brief Shuts the progam down.
 * @param hModule The thread's HMODULE.
 * @param fstdout The stdout handle.
 * @param exitCode Exit code, default 0.
 */
void __stdcall shutdown(HMODULE hModule, FILE* fstdout, DWORD exitCode = 0) {
  if (fstdout != nullptr) fclose(fstdout);
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
  MessageBoxA(0, message.c_str(), "phobos panicked!", MB_OK | MB_ICONERROR);
  shutdown(hModule, fstdout, 1);

  exit(1);
}

s32* pointerPath(std::vector<std::ptrdiff_t> offsets, std::intptr_t baseAddress) {
  std::intptr_t temp = baseAddress;

  for (size_t i = 0; i < offsets.size() - 1; i++) {
    temp = *reinterpret_cast<std::intptr_t*>(temp + offsets[i]);
  }

  return (reinterpret_cast<s32*>(temp + offsets.back()));
}

/**
 * @brief Gets game values.
 * @return Game values.
 */
GameValues getGameValues() {
  std::intptr_t baseAddress = reinterpret_cast<std::intptr_t>(GetModuleHandle(NULL));

  // replacing everything w the correct values :3  
  volatile s32* coins = pointerPath({ 0xEFFC4, 0x58, 0x24, 0xC, 0x10, 0x3C, 0x84 }, baseAddress);
  volatile s32* fertilizer = pointerPath({ 0x13A90C, 0x548, 0x274, 0x18, 0x0, 0x0, 0x4, 0x254 }, baseAddress);
  volatile s32* bugSpray = pointerPath({ 0x13A90C, 0x54C, 0x110, 0xC, 0x258 }, baseAddress);
  volatile s32* chocolate = pointerPath({ 0x13A90C, 0x548, 0x25C, 0x28, 0x70, 0x8, 0x4, 0x284 }, baseAddress);

  return { .coins = coins, .chocolate = chocolate, .fertilizer = fertilizer, .bugSpray = bugSpray };
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

  SetConsoleTitleA("phobos");

  if (fstdout == nullptr) {
    panic("failed to open stdout!", hModule);
  }

  // the main loop ft. horrible ansi stuff
  bool enabled = false;  

  std::cout << "\x1b[38;5;252m";

  GameValues values = getGameValues();
  while (!GetAsyncKeyState('Q')) {
    std::cout << *values.coins << "    \r";
  }

  shutdown(hModule, fstdout);
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)attachedMain, hModule, 0, NULL);
  }

  return 1;
}