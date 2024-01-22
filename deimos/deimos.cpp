#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void __stdcall attachedMain(HMODULE hModule) {

}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    MessageBoxA(0, "hi", "hi", MB_OK);
  }

  return 1;
}