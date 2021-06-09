#include <iostream>
#include "includes.h"
#include  "offsets.h"
#include "KeInterface.h"




int main()
{
	KeInterface Driver("\\\\.\\kernelhop");

	DWORD ProcessId = Driver.GetTargetPid();
	DWORD ClientAddress = Driver.GetClientModule();

	DWORD LocalPlayer = Driver.ReadVirtualMemory<DWORD>(ProcessId, ClientAddress + LOCAL_PLAYER, sizeof(ULONG));

	DWORD InGround = Driver.ReadVirtualMemory<DWORD>(ProcessId,
		LocalPlayer + FFLAGS, sizeof(ULONG));



	std::cout << "Encontrado csgo Process Id: " << ProcessId << std::endl;
	std::cout << "Encontrado client.dll ClientBase: 0x" << std::uppercase
		<< std::hex << ClientAddress << std::endl;
	std::cout << "Encontrado LocalPlayer in client.dll: 0x" << std::uppercase
		<< std::hex << LocalPlayer << std::endl;
	std::cout << "Encontrado PlayerInGround: 0x" << std::uppercase
		<< std::hex << InGround << std::endl;

	while (true)
	{
		// checar player no chão
		DWORD InGround = Driver.ReadVirtualMemory<DWORD>(ProcessId, LocalPlayer + FFLAGS, sizeof(ULONG));
		// checar espaço
		if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && (InGround & 1 == 1))
		{
			// pular
			Driver.WriteVirtualMemory(ProcessId, ClientAddress + FORCE_JUMP, 0x5, 8);
			Sleep(50);
			// resetar
			Driver.WriteVirtualMemory(ProcessId, ClientAddress + FORCE_JUMP, 0x4, 8);
			
		}
		Sleep(10);
	}
    return 0;
}

//Trigger

HWND hwnd;
DWORD procId;
HANDLE hProcess;
uintptr_t moduleBase;
HDC hdc;
int closest;
uintptr_t GetModuleBaseAddress(const char* modName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!strcmp(modEntry.szModule, modName))
				{
					CloseHandle(hSnap);
					return (uintptr_t)modEntry.modBaseAddr;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
}

template<typename T> T RM(SIZE_T address)
{
	T buffer;
	ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

uintptr_t getLocalPlayer()
{
	return RM<uintptr_t>(moduleBase + dwLocalPlayer);
}


uintptr_t getLocal(int index)
{
	return RM<uintptr_t>(moduleBase + dwEntityList + index * 0x10);
}


int getTeam(uintptr_t player)
{
	return RM<int>(player + m_iTeamNum);
}


int getCrosshairID(uintptr_t player)
{
	return RM<int>(player + m_iCrosshairId);
}
void trigger(void)
{
	while (1)
	{
		int CrosshairID = getCrosshairID(getLocalPlayer());
		int CrosshairTeam = getTeam(getLocal(CrosshairID - 1));
		int LocalTeam = getTeam(getLocalPlayer());
		if (CrosshairID > 0 && CrosshairID < 32 && LocalTeam != CrosshairTeam)
		{
			if (GetAsyncKeyState(0x06))
			{
				mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
				Sleep(70);
			}
		}
	}
}

int main(void)
{
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hwnd, &procId);
	moduleBase = GetModuleBaseAddress("client.dll");
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	trigger();
}


