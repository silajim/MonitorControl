#ifndef PRIMARYMONITOR_H
#define PRIMARYMONITOR_H

#include <Windows.h>
#include <vector>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <atlstr.h> // CW2A
#include <PhysicalMonitorEnumerationAPI.h>

#include <string>

class PrimaryMonitor
{
public:
    PrimaryMonitor();
    ~PrimaryMonitor();

    bool canAdjustBrightness() const;
    unsigned char getCurrentBrightness() const;
    void setBrightness(DWORD brigthness);
    DWORD getLastBigthness() const;


private:

    void refreshHandle();

    void getPhysicalMonitors(HMONITOR hMonitor, std::vector<PHYSICAL_MONITOR> &Monitorsvec);

    int capabilities(PHYSICAL_MONITOR hmonitor);
    static bool getPathInfo(const MONITORINFOEXW &viewInfo, DISPLAYCONFIG_PATH_INFO *pathInfo);

    bool GetMonitorCap();
    bool getPrimaryMonitor();
    bool getMonitorName();
    bool getDisplayInfo(DISPLAY_DEVICEW &ddw);
    bool getEdid();
    std::wstring GetHKEY(HKEY key);
    HMONITOR hmonitor;
    DWORD dwMonitorCapabilities = 0;
    DWORD dwSupportedColorTemperatures = 0;
    std::wstring monitorFirendlyName;
    DISPLAY_DEVICEW DisplayInfo;    
    std::vector<PHYSICAL_MONITOR> physicalMonitors{};
    PHYSICAL_MONITOR pmonitor;


    DWORD lastBigthness;

    DWORD minBrightness;
    DWORD maxBrightness;

};

#endif // PRIMARYMONITOR_H
