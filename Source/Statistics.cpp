//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-01 04:11:15
//

#include <Statistics.hpp>
#include <Windows.h>
#include <Psapi.h>

void Statistics::Update()
{
    Statistics& stats = Get();

    // Ram
    {
        HANDLE hProcess = GetCurrentProcess();

        PROCESS_MEMORY_COUNTERS pmc;
        GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));

        UInt64 totalRam = 0;
        GetPhysicallyInstalledSystemMemory(&totalRam);

        stats.UsedRAM = pmc.WorkingSetSize;
        stats.MaxRAM = totalRam * 1024;
    }

    // Battery
    {
        SYSTEM_POWER_STATUS status;
        GetSystemPowerStatus(&status);

        stats.Battery = status.BatteryLifePercent;
    }
}