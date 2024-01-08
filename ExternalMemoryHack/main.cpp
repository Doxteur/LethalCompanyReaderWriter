#include <Windows.h>
#include <vector>
#include <iostream>
#include <TlHelp32.h>



using namespace std;
uintptr_t unityAddress = 0;

DWORD GetProcessIdFromWindow(HWND hwnd) {
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    return processId;
}

void ListModules(DWORD processId) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Erreur lors de la création du cliché des modules" << std::endl;
        return;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snapshot, &moduleEntry)) {
        do {
            if (_wcsicmp(moduleEntry.szModule, L"UnityPlayer.dll") == 0) {
                unityAddress = (uintptr_t)moduleEntry.modBaseAddr;
            }
        } while (Module32Next(snapshot, &moduleEntry));
    }
    else {
        std::cerr << "Erreur lors de l'obtention des informations sur les modules" << std::endl;
    }

    CloseHandle(snapshot);
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t buffer[256];
    GetWindowTextW(hwnd, buffer, sizeof(buffer) / sizeof(buffer[0]));

    if (wcsstr(buffer, L"Lethal Company") != nullptr) {
        DWORD processId = GetProcessIdFromWindow(hwnd);
        ListModules(processId);

        return FALSE; // Arrêter l'énumération car vous avez trouvé la fenêtre
    }

    return TRUE; // Continuer l'énumération
}




int main(int argc, char** argv) {


    EnumWindows(EnumWindowsProc, 0);
    cout << "unityAddress: " << unityAddress << endl;

    HWND lethal_company_window = FindWindow(NULL, L"Lethal Company");
    std::cout << "Lethal Company Window: " << lethal_company_window << std::endl;

    DWORD process_id = 0;

    GetWindowThreadProcessId(lethal_company_window, &process_id);

    HANDLE lethal_company_process = OpenProcess(PROCESS_ALL_ACCESS, true, process_id);
    if (lethal_company_process == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    DWORD dllBase = (DWORD)GetModuleHandle(L"UnityPlayer.dll");

    // Adresse de base de "UnityPlayer.dll"
    uintptr_t gold_address = 0;
    
    std::vector<DWORD> offsets = { 0x110, 0xC0, 0x1C8, 0x1B0, 0xB0, 0x1F4 };
    
    SIZE_T gold_value = 0;
    SIZE_T test = 0;

    SIZE_T bytes_readValue = 0;

    ReadProcessMemory(lethal_company_process, (LPCVOID)(unityAddress + 0x01BE9D00), &gold_value, sizeof(gold_value), &bytes_readValue);

    for (int i = 0; i < offsets.size(); i++) {
        SIZE_T current_address = gold_value + offsets[i];
        gold_address = current_address;
        ReadProcessMemory(lethal_company_process, (LPCVOID)current_address, &gold_value, sizeof(gold_value), &bytes_readValue);
    }

    cout << "Montant Actuel: " << gold_value << endl;

    DWORD new_gold_value = 0;
    SIZE_T bytes_written = 0;

    cout << "Choisir un montant: ";
    cin >> new_gold_value;

    if (gold_address != 0) {
        WriteProcessMemory(lethal_company_process, (LPVOID)gold_address, &new_gold_value, sizeof(new_gold_value), &bytes_written);
	}

    return 0;
}
