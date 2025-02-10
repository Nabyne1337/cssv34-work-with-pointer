#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <conio.h>
#include <chrono>
#include <thread>
using namespace std;

HWND hwnd;
DWORD procID;
HANDLE hProcess;

uintptr_t ModuleBase;
uintptr_t PlayerBase;

int m_iHealth = 0;

uintptr_t GetModuleBaseAddress(const wchar_t* modName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32FirstW(hSnap, &modEntry)) {
            do {
                if (!wcscmp(modEntry.szModule, modName)) {
                    CloseHandle(hSnap);
                    return reinterpret_cast<uintptr_t>(modEntry.modBaseAddr);
                }
            } while (Module32NextW(hSnap, &modEntry));
        }
        CloseHandle(hSnap);
    }
    return 0;
}

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}

void Health_monitor() {
    uintptr_t dynamicptrbaseaddr = ModuleBase + 0x004035C0;
    std::vector<unsigned int> speedoffset = { 0x5C };

    uintptr_t myAddr = FindDMAAddy(hProcess, dynamicptrbaseaddr, speedoffset);

    ReadProcessMemory(hProcess, (LPVOID)myAddr, &m_iHealth, sizeof(m_iHealth), 0);
    /*WriteProcessMemory(hProcess, (LPVOID)myAddr, &speed, sizeof(speed), 0);*/
    std::cout << "Health: " << m_iHealth << std::endl;
}

int main() {

    SetConsoleTitle(L"onetap v2");

    hwnd = FindWindow(NULL, L"Counter-Strike Source");
    if (hwnd == NULL) {
        cout << "[-] Please Open the CSS V34" << endl;
        Sleep(1000);
        return 0;
    }

    procID = 0;
    GetWindowThreadProcessId(hwnd, &procID);

    if (procID == 0) {
        cout << "\n\n [-] Process ID not Found" << endl;
        return 0;
    }

    ModuleBase = GetModuleBaseAddress(L"client.dll");
    PlayerBase = GetModuleBaseAddress(L"server.dll");
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    cout << "\n\n [+] Process hl2.exe is Opened" << endl;
    cout << "\n\n Press enter..." << endl;
    _getch();

    while (true) {
        system("cls");
        Health_monitor();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    _getch();
    return 0;

}
