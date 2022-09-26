#ifndef MONITORCONTROLLER_H
#define MONITORCONTROLLER_H

#include <Windows.h>
#include <vector>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <atlstr.h> // CW2A
#include <PhysicalMonitorEnumerationAPI.h>

#include <string>

class MonitorController
{
public:
    MonitorController();
    ~MonitorController();
    void getPhysicalMonitors(HMONITOR hMonitor, std::vector<PHYSICAL_MONITOR> &Monitorsvec);

    int capabilities(PHYSICAL_MONITOR hmonitor);
    static bool getPathInfo(const MONITORINFOEXW &viewInfo, DISPLAYCONFIG_PATH_INFO *pathInfo);

private:
    bool GetMonitorCap();
    bool getPrimaryMonitor();
    bool getMonitorName();
    bool getDisplayInfo();
    bool getEdid();
    std::wstring GetHKEY(HKEY key);
    HMONITOR hmonitor;
    DWORD dwMonitorCapabilities = 0;
    DWORD dwSupportedColorTemperatures = 0;
    std::wstring monitorFirendlyName;
    DISPLAY_DEVICEW DisplayInfo;    
    std::vector<PHYSICAL_MONITOR> physicalMonitors{};
};

#endif // MONITORCONTROLLER_H
