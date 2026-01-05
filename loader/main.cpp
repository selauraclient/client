#include <Windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <print>
#include <thread>
#include <chrono>
#include <filesystem>
#include <Aclapi.h>
#include <Sddl.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Advapi32.lib")

std::string getLastErrorAsString() {
    DWORD errorMessageID = GetLastError();
    if (errorMessageID == 0) return "";
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

int setPermissions(const std::string& filePath) {
    PACL curDACL = nullptr;
    PSECURITY_DESCRIPTOR desc = nullptr;

    if (GetNamedSecurityInfoA(filePath.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &curDACL, nullptr, &desc) != ERROR_SUCCESS) {
        return 6;
    }

    PSID sid = nullptr;
    if (!ConvertStringSidToSidA("S-1-15-2-1", &sid)) {
        LocalFree(desc);
        return 7;
    }

    EXPLICIT_ACCESS_A access{};
    access.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
    access.grfAccessMode = SET_ACCESS;
    access.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    access.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    access.Trustee.ptstrName = (LPCH)sid;

    PACL newDACL = nullptr;
    if (SetEntriesInAclA(1, &access, curDACL, &newDACL) != ERROR_SUCCESS) {
        LocalFree(sid);
        LocalFree(desc);
        return 8;
    }

    int result = 0;
    if (SetNamedSecurityInfoA(const_cast<char*>(filePath.c_str()), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, newDACL, nullptr) != ERROR_SUCCESS) {
        result = 9;
    }

    LocalFree(sid);
    LocalFree(newDACL);
    LocalFree(desc);
    return result;
}

DWORD findMinecraftPID() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W entry{ sizeof(entry) };
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"Minecraft.Windows.exe") == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

int inject(DWORD pid, std::string dllPath) {
    char fullPath[MAX_PATH];
    GetFullPathNameA(dllPath.c_str(), MAX_PATH, fullPath, nullptr);

    if (!PathFileExistsA(fullPath)) {
        std::println("DLL Path does not exist: {}", fullPath);
        return 2;
    }

    if (int err = setPermissions(fullPath)) {
        std::println("Warning: Permission set failed (Code {}). Injection might fail.", err);
    }

    const auto handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!handle) return 1;

    size_t length = strlen(fullPath) + 1;
    const auto address = VirtualAllocEx(handle, nullptr, length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!address) {
        CloseHandle(handle);
        return 3;
    }

    if (!WriteProcessMemory(handle, address, fullPath, length, nullptr)) {
        VirtualFreeEx(handle, address, 0, MEM_RELEASE);
        CloseHandle(handle);
        return 4;
    }

    const auto thread = CreateRemoteThread(handle, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, address, 0, nullptr);
    if (!thread) {
        std::println("Remote Thread Failed: {}", getLastErrorAsString());
        VirtualFreeEx(handle, address, 0, MEM_RELEASE);
        CloseHandle(handle);
        return 5;
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    VirtualFreeEx(handle, address, 0, MEM_RELEASE);
    CloseHandle(handle);

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("Usage: SelauraInjector.exe <path_to_dll>");
        return 1;
    }

    std::string dllTarget = argv[1];
    DWORD pid = 0;

    std::println("Searching for Minecraft...");
    pid = findMinecraftPID();

    if (pid == 0) {
        std::println("Minecraft not found. Attempting to launch...");
        ShellExecuteW(nullptr, L"open", L"minecraft://", nullptr, nullptr, SW_SHOWNORMAL);

        auto start = std::chrono::steady_clock::now();
        while (pid == 0) {
            if (std::chrono::steady_clock::now() - start > std::chrono::seconds(20)) {
                std::println("Timed out waiting for Minecraft to start.");
                return 1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            pid = findMinecraftPID();
        }
    }

    std::println("Found Minecraft (PID: {})", pid);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    int result = inject(pid, dllTarget);
    if (result != 0) {
        std::println("Injection failed with code: {}", result);
        return 1;
    }

    std::println("Successfully injected!");
    return 0;
}