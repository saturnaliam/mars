#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <cstdint>
#include <vector>
#include <optional>

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
void __stdcall shutdown(HMODULE hModule, FILE* fstdout, FILE* fstdin, DWORD exitCode = 0) {
  if (fstdout != nullptr) fclose(fstdout);
  if (fstdin != nullptr) fclose(fstdin);
  FreeConsole();
  FreeLibraryAndExitThread(hModule, exitCode);
}

/**
 * @brief Panics and ends the program.
 * @param message The message.
 * @param hModule HMODULE of the thread.
 * @param fstdout The stdout handle, default nullptr.
 */
void __stdcall panic(std::string message, HMODULE hModule, FILE* fstdout = nullptr, FILE* fstdin = nullptr) {
  MessageBoxA(0, message.c_str(), "phobos panicked!", MB_OK | MB_ICONERROR);
  shutdown(hModule, fstdout, fstdin, 1);

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

std::optional<s32> userInputToInt() {
  std::string input;
  std::cin.clear();
  std::cin >> input; 
  
  if (input == "") return {};

  for (char c : input) {
    if (!isdigit(c)) {
      return {};
    }
  }

  return atoi(input.c_str());
}

void __stdcall attachedMain(HMODULE hModule) {
  // setting up the console
  AllocConsole();
  FILE* fstdout;
  FILE* fstdin;
  freopen_s(&fstdout, "CONOUT$", "w", stdout);
  freopen_s(&fstdin, "CONIN$", "r", stdin);

  // setting up ANSI escape codes
  HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD consoleMode;
  GetConsoleMode( handleOut , &consoleMode);
  consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;            
  SetConsoleMode( handleOut , consoleMode );

  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(handleOut, &cursorInfo);
  cursorInfo.bVisible = false;
  SetConsoleCursorInfo(handleOut, &cursorInfo);

  SetConsoleTitleA("phobos");

  if (fstdout == nullptr || fstdin == nullptr) {
    panic("failed to open stdout!", hModule);
  }

  // the main loop ft. horrible ansi stuff
  bool enabled = false;  

  std::cout << R"(     _       _           
 ___| |_ ___| |_ ___ ___ 
| . |   | . | . | . |_ -|
|  _|_|_|___|___|___|___|
|_| by saturnalia )";

  std::cout << "\x1b[38;5;252m";
  std::cout << "\n\nEdit Coins [C] // Edit Fertilizer [F] // Edit Bug Spray [B] // Edit Chocolate [H] // Quit [Q]\n\n";

  GameValues values = getGameValues();
  while (!GetAsyncKeyState('Q')) {
    std::cout << "\x1b[H\n\n\n\n\n\n\n\n";
    printf("\x1b[2KCoins: %d @ %X (in game as %d)\n", *values.coins, values.coins, *values.coins*10);
    printf("Fertilizer: %d @ %X (in game as %d)   \n", *values.fertilizer, values.fertilizer, *values.fertilizer-1000);
    printf("Bug Spray: %d @ %X (in game as %d)   \n", *values.bugSpray, values.bugSpray, *values.bugSpray-1000);
    printf("Chocolate: %d @ %X (in game as %d)   \n", *values.chocolate, values.chocolate, *values.chocolate-1000);

    if (GetAsyncKeyState('C')) {
      std::cout << "Editing coins: ";
      std::cin.clear();
      std::optional<int> input = userInputToInt();
      *values.coins = input.value_or(*values.coins);
      std::cout << "\x1b[A\x1b[2K";
    } else if (GetAsyncKeyState('F')) {
      std::cout << "Editing fertilizer: ";
      std::cin.clear();
      std::optional<int> input = userInputToInt();
      *values.fertilizer = input.value_or(*values.fertilizer);
      std::cout << "\x1b[A\x1b[2K";
    } else if (GetAsyncKeyState('B')) {
      std::cout << "Editing bug spray: ";
      std::cin.clear();
      std::optional<int> input = userInputToInt();
      *values.bugSpray = input.value_or(*values.bugSpray);
      std::cout << "\x1b[A\x1b[2K";
    } else if (GetAsyncKeyState('H')) {
      std::cout << "Editing chocolate: ";
      std::cin.clear();
      std::optional<int> input = userInputToInt();
      *values.chocolate = input.value_or(*values.chocolate);
      std::cout << "\x1b[A\x1b[2K";
    }
  }

  shutdown(hModule, fstdout, fstdin);
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)attachedMain, hModule, 0, NULL);
  }

  return 1;
}