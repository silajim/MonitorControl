#include "primarymonitor.h"

#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <Windows.h>
#include <WinUser.h>

#include <setupapi.h>
#include <devguid.h>
#include <initguid.h>

#include <wchar.h>
#include <iomanip>

#include "WindowsErrorException.h"

//#include <devguid.h>
//#include <Ntddvdeo.h>
#define InvalidMonitorCapString 3223725447
#define InvalidDDCIMessage 3223725449
#define InvalidDDCIValue 3223725450
#define CheckSum_Error 3223725451
#define InvalidMonitorHandle  3223725452
#define MonitorHanldeDestroyed 3223725453

PrimaryMonitor::PrimaryMonitor()
{
    //    std::vector<PHYSICAL_MONITOR> physicalMonitors{};

    if(!getPrimaryMonitor()){
        return;
    }

    try{
        getPhysicalMonitors(hmonitor,physicalMonitors);
    }catch(WindowsErrorException e){
        std::wcout << "getPhysicalMonitors Error code " << e.errorCode() << std::endl;
        throw e;
    }

    if(physicalMonitors.size()==0){
        return ;
    }



    pmonitor = physicalMonitors[0];
    //    std::wcout << "PY desc " << pmonitor.szPhysicalMonitorDescription << std::endl;

    MONITORINFOEXW monitorInfo;
    memset(&monitorInfo, 0, sizeof(MONITORINFOEXW));
    monitorInfo.cbSize = sizeof(MONITORINFOEXW);
    BOOL bSuccess;

    do{
        bSuccess = GetMonitorInfoW(hmonitor,&monitorInfo);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);

    height = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
    width = monitorInfo.rcWork.right - monitorInfo.rcWork.left;

    posy = monitorInfo.rcWork.top;
    posx = monitorInfo.rcWork.left;

    std::wcout << "Monitor DEVICENAME " << monitorInfo.szDevice << std::endl;

    try{
        GetMonitorCap();
    }catch(WindowsErrorException e){
        std::wcout << "GetMonitorCap Error code " << e.errorCode() << std::endl;
        throw e;
    }


    if(dwMonitorCapabilities & MC_CAPS_NONE ){
        std::wcout << "NO VCP Capabilities" << std::endl;
        return ;
    }

    //        CheckFlag(dwMonitorCapabilities,MC_CAPS_BRIGHTNESS);
    //        CheckFlag(dwMonitorCapabilities,MC_CAPS_CONTRAST);
    //        CheckFlag(dwMonitorCapabilities,MC_CAPS_COLOR_TEMPERATURE);

    //        CheckFlag(dwSupportedColorTemperatures,MC_SUPPORTED_COLOR_TEMPERATURE_NONE);
    //        CheckFlag(dwSupportedColorTemperatures,MC_SUPPORTED_COLOR_TEMPERATURE_4000K);
    //        CheckFlag(dwSupportedColorTemperatures,MC_SUPPORTED_COLOR_TEMPERATURE_5000K);
    //        CheckFlag(dwSupportedColorTemperatures,MC_SUPPORTED_COLOR_TEMPERATURE_6500K);



    try{
        getMonitorName();
    }catch(WindowsErrorException e){
        std::wcout << "getMonitorName Error code " << e.errorCode() << std::endl;
        throw e;
    }

    DISPLAY_DEVICEW ddw;
    try{
        getDisplayInfo(ddw);
    }catch(WindowsErrorException e){
        std::wcout << "getDisplayInfo Error code " << e.errorCode() << std::endl;
        throw e;
    }

    DisplayInfo = ddw;

    try{
        getEdid();
    }catch(WindowsErrorException e){
        std::wcout << "getEdid Error code " << e.errorCode() << std::endl;
        throw e;
    }

    DWORD currBrightness;

    do{
        bSuccess = GetMonitorBrightness(hmonitor,&minBrightness,&currBrightness,&maxBrightness);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);

    //    DestroyPhysicalMonitors(physicalMonitors.size(), physicalMonitors.data());

}

PrimaryMonitor::~PrimaryMonitor()
{
    DestroyPhysicalMonitors(physicalMonitors.size(), physicalMonitors.data());
    DestroyPhysicalMonitor(pmonitor.hPhysicalMonitor);
}

bool PrimaryMonitor::canAdjustBrightness() const
{
    return dwMonitorCapabilities & MC_CAPS_BRIGHTNESS;
}

unsigned char PrimaryMonitor::getCurrentBrightness() const
{
    if(!(dwMonitorCapabilities & MC_CAPS_BRIGHTNESS)){
        return -1;
    }

    DWORD currBrightness;
    DWORD b1,b2;
    BOOL bSuccess;

    do{
        bSuccess = GetMonitorBrightness(hmonitor,&b1,&currBrightness,&b2);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);

    return static_cast<unsigned char>(currBrightness);
}

void PrimaryMonitor::setBrightness(DWORD brigthness)
{
    BOOL success;
    do{
        success = SetMonitorBrightness(pmonitor.hPhysicalMonitor,brigthness);
    }while(success==FALSE && GetLastError() == CheckSum_Error);

    std::wcout << L"setBrightness " << GetLastError() << std::endl;

    if(success == FALSE && (GetLastError() == InvalidMonitorHandle || GetLastError() == MonitorHanldeDestroyed)){
        refreshHandle();
        setBrightness(brigthness);
    }

    if(success==TRUE){
        lastBigthness = brigthness;
    }else{
        throw WindowsErrorException(GetLastError());
    }
}

void PrimaryMonitor::refreshHandle()
{
    //    std::vector<PHYSICAL_MONITOR> physicalMonitors{};

    DestroyPhysicalMonitors(physicalMonitors.size(), physicalMonitors.data());
    physicalMonitors.clear();

    if(!getPrimaryMonitor()){
        return;
    }

    try{
        getPhysicalMonitors(hmonitor,physicalMonitors);
    }catch(WindowsErrorException e){
        std::wcout << L"getPhysicalMonitors Error code " << e.errorCode() << std::endl;
        throw e;
    }

    if(physicalMonitors.size()==0){
        return ;
    }

    if(dwMonitorCapabilities & MC_CAPS_NONE ){
        std::wcout << L"NO VCP Capabilities" << std::endl;
        return ;
    }

    DISPLAY_DEVICEW ddw;

    MONITORINFOEXW monitorInfo;
    memset(&monitorInfo, 0, sizeof(MONITORINFOEXW));
    monitorInfo.cbSize = sizeof(MONITORINFOEXW);
    BOOL bSuccess;

    do{
        bSuccess = GetMonitorInfoW(hmonitor,&monitorInfo);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);


    try{
        getDisplayInfo(ddw);
    }catch(WindowsErrorException e){
        std::wcout << L"getDisplayInfo Error code " << e.errorCode() << std::endl;
        throw e;
    }

    if(wcscmp(ddw.DeviceID,DisplayInfo.DeviceID)!=0){
        return;
    }

    pmonitor = physicalMonitors[0];
    DisplayInfo = ddw;

    //    try{
    //        GetMonitorCap();
    //    }catch(WindowsErrorException e){
    //        std::wcout << L"GetMonitorCap Error code " << e.errorCode() << std::endl;
    //        throw e;
    //    }


    //    try{
    //        getMonitorName();
    //    }catch(WindowsErrorException e){
    //        std::wcout << L"getMonitorName Error code " << e.errorCode() << std::endl;
    //        throw e;
    //    }

    //    try{
    //        getEdid();
    //    }catch(WindowsErrorException e){
    //        std::wcout << L"getEdid Error code " << e.errorCode() << std::endl;
    //        throw e;
    //    }

    //    DestroyPhysicalMonitors(physicalMonitors.size(), physicalMonitors.data());
}

void PrimaryMonitor::getPhysicalMonitors(HMONITOR hMonitor, std::vector<PHYSICAL_MONITOR> &Monitorsvec){
    DWORD numberOfPhysicalMonitors;
    BOOL success = GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numberOfPhysicalMonitors);
    if (success) {
        auto originalSize = Monitorsvec.size();
        Monitorsvec.resize(originalSize + numberOfPhysicalMonitors);
        success = GetPhysicalMonitorsFromHMONITOR(hMonitor, numberOfPhysicalMonitors, Monitorsvec.data() + originalSize);
    }
}

int PrimaryMonitor::capabilities(PHYSICAL_MONITOR hmonitor) {


    //    size_t displayId = INT_MAX;

    //    if (displayId > physicalMonitors.size() - 1) {
    //        std::cerr << "Invalid display ID" << std::endl;
    //        return 1;
    //    }

    auto physicalMonitorHandle =hmonitor.hPhysicalMonitor;  //physicalMonitors[displayId].hPhysicalMonitor;

    DWORD capabilitiesStringLengthInCharacters;
    BOOL success;
    do{
        success =  GetCapabilitiesStringLength(physicalMonitorHandle, &capabilitiesStringLengthInCharacters);
    } while(success==FALSE && GetLastError() == CheckSum_Error);
    if (!success) {
        std::cerr << "Failed to get capabilities string length" << std::endl;
        std::cerr << "Error: " << GetLastError() << std::endl;
        throw WindowsErrorException(GetLastError());
        return 1;
    }

    std::unique_ptr<char[]> capabilitiesString{ new char[capabilitiesStringLengthInCharacters] };
    do{
        success = CapabilitiesRequestAndCapabilitiesReply(physicalMonitorHandle, capabilitiesString.get(), capabilitiesStringLengthInCharacters);
    }while(success==FALSE && GetLastError() == CheckSum_Error);
    if (!success) {
        std::cerr << "Failed to get capabilities string" << std::endl;
        throw WindowsErrorException(GetLastError());
    }

    std::cout << std::string(capabilitiesString.get()) << std::endl;

    return 0;
}


bool PrimaryMonitor::getPathInfo(const MONITORINFOEXW &viewInfo, DISPLAYCONFIG_PATH_INFO *pathInfo)
{
    // We might want to consider storing adapterId/id from DISPLAYCONFIG_PATH_TARGET_INFO.
    std::vector<DISPLAYCONFIG_PATH_INFO> pathInfos;
    std::vector<DISPLAYCONFIG_MODE_INFO> modeInfos;

    // Fetch paths
    LONG result;
    UINT32 numPathArrayElements;
    UINT32 numModeInfoArrayElements;
    do {
        // QueryDisplayConfig documentation doesn't say the number of needed elements is updated
        // when the call fails with ERROR_INSUFFICIENT_BUFFER, so we need a separate call to
        // look up the needed buffer sizes.
        if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, &numModeInfoArrayElements) != ERROR_SUCCESS) {
            throw WindowsErrorException(GetLastError());
            return false;
        }
        pathInfos.resize(numPathArrayElements);
        modeInfos.resize(numModeInfoArrayElements);
        result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, pathInfos.data(),&numModeInfoArrayElements, modeInfos.data(), nullptr);
    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS){
        throw WindowsErrorException(GetLastError());
        return false;
    }

    // Find path matching monitor name
    for (uint32_t p = 0; p < numPathArrayElements; p++) {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME deviceName;
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        deviceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
        deviceName.header.adapterId = pathInfos[p].sourceInfo.adapterId;
        deviceName.header.id = pathInfos[p].sourceInfo.id;
        if (DisplayConfigGetDeviceInfo(&deviceName.header) == ERROR_SUCCESS) {
            if (wcscmp(viewInfo.szDevice, deviceName.viewGdiDeviceName) == 0) {
                *pathInfo = pathInfos[p];
                return true;
            }
        }else{
            throw WindowsErrorException(GetLastError());
        }
    }

    return false;
}

bool PrimaryMonitor::getPrimaryMonitor()
{
    const POINT ptZero = { 0, 0 };
    hmonitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    if(hmonitor==NULL)
        return false;
    return true;
}

bool PrimaryMonitor::getMonitorName()
{
    MONITORINFOEXW monitorInfo;
    memset(&monitorInfo, 0, sizeof(MONITORINFOEXW));
    monitorInfo.cbSize = sizeof(MONITORINFOEXW);
    BOOL bSuccess;

    do{
        bSuccess = GetMonitorInfoW(hmonitor,&monitorInfo);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);

    if(!bSuccess){
        throw WindowsErrorException(GetLastError());
    }

    DISPLAYCONFIG_PATH_INFO pathInfo = {};
    const bool hasPathInfo = getPathInfo(monitorInfo, &pathInfo);
    if (hasPathInfo) {
        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName = {};
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
        deviceName.header.adapterId = pathInfo.targetInfo.adapterId;
        deviceName.header.id = pathInfo.targetInfo.id;
        if (DisplayConfigGetDeviceInfo(&deviceName.header) == ERROR_SUCCESS)
            monitorFirendlyName = deviceName.monitorFriendlyDeviceName;
        else
            throw WindowsErrorException(GetLastError());
    }

    //    std::wcout << "target id " << pathInfo.targetInfo.id << std::endl;
    //    std::wcout << "Name from QT " << monitorFirendlyName << std::endl;

    return true;
}

bool PrimaryMonitor::getDisplayInfo(DISPLAY_DEVICEW &ddw)
{
    //    DISPLAY_DEVICEW ddw;
    ddw.cb = sizeof(DISPLAY_DEVICEW);
    BOOL bSuccess;

    do{
        bSuccess = EnumDisplayDevicesW(NULL,0,&ddw,0);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);

    if(!bSuccess)
        throw WindowsErrorException(GetLastError());
    //        return false;

    std::wstring deviceName = ddw.DeviceName;

    do{
        bSuccess = EnumDisplayDevicesW(deviceName.c_str(),0,&ddw,EDD_GET_DEVICE_INTERFACE_NAME);
    }while(bSuccess==FALSE && GetLastError() == CheckSum_Error);

    if(!bSuccess)
        throw WindowsErrorException(GetLastError());

    std::wcout << L"Device Name " << ddw.DeviceName << std::endl;
    std::wcout << L"Device String " << ddw.DeviceString << std::endl;
    std::wcout << L"Device ID " << ddw.DeviceID << std::endl;
    std::wcout << L"Device KEY " << ddw.DeviceKey << std::endl;

    //    DisplayInfo = ddw;

    return true;
}

bool PrimaryMonitor::getEdid()
{

    // FROM https://github.com/GKO95/Win32.EDID/blob/master/EDID.cpp/main.cpp
    // AND
    // FROM https://ofekshilon.com/2014/06/19/reading-specific-monitor-dimensions/

    //    GUID* ptrGUID = NULL;	// pointer to the GUID container.

    GUID GUID_DEVINTERFACE_MONITOR = { 0xe6f07b5f, 0xee97, 0x4a90, 0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7 };

    HDEVINFO devINFO = INVALID_HANDLE_VALUE;
    devINFO = SetupDiGetClassDevs(&GUID_DEVINTERFACE_MONITOR, NULL, NULL, DIGCF_DEVICEINTERFACE);

    if (devINFO == INVALID_HANDLE_VALUE)
    {
        MessageBoxW(NULL, L"Failed to retrieve device information of the GUID class.", L"", MB_ICONERROR);
        throw WindowsErrorException(GetLastError());
    }

    /*
        SP_DEVINFO_DATA is a structure for a single device instance of the HDEVINFO device information set.
        Here, variable "devDATA" is used to temporary store a single device information from "devINFO" data.
        SP_DEVINFO_DATA.cbSize field must be specified as function that accpets SP_DEVINFO_DATA
        always check whether the byte size of the SP_DEVINFO_DATA structure is the same as the cbSize field.
        REFERENCE: https://docs.microsoft.com/en-us/windows/win32/api/setupapi/ns-setupapi-sp_devinfo_data
    */
    SP_DEVINFO_DATA devDATA;
    devDATA.cbSize = sizeof(SP_DEVINFO_DATA);

    BOOL devFOUND = TRUE;
    for (DWORD index = 0; devFOUND; index++)
    {
        SP_DEVICE_INTERFACE_DATA did;
        did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);


        if(!SetupDiEnumDeviceInterfaces(devINFO,NULL,&GUID_DEVINTERFACE_MONITOR,index,&did)){
            throw WindowsErrorException(GetLastError());
            //            std::wcout <<"ERROR " << GetLastError() << std::endl;
            //            fflush(stdout);
        }

        DWORD reqsize;

        SetupDiGetDeviceInterfaceDetailW(devINFO, &did, NULL, 0, &reqsize, NULL);
        DWORD offset = offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA_W, DevicePath) + sizeof(WCHAR);


        SP_DEVICE_INTERFACE_DETAIL_DATA_W *did_data = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)malloc(reqsize+offset);
        did_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        SetupDiGetDeviceInterfaceDetailW(devINFO, &did, did_data, reqsize, NULL, NULL);


        std::wcout << L"DEVPATH " <<  did_data->DevicePath << std::endl;

        if( 0 == _wcsicmp( did_data->DevicePath, DisplayInfo.DeviceID ) )
        {
            SetupDiEnumDeviceInfo(devINFO,index, &devDATA);
            HKEY devKEY =SetupDiOpenDevRegKey(devINFO, &devDATA, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ );
            wprintf_s(L"Registry Key 0: \"%s\"\n", GetHKEY(devKEY).c_str());

            BYTE byteBUFFER[128] = { 0 };
            DWORD regSize = 128;
            DWORD regType = REG_BINARY;
            LRESULT lResult = RegQueryValueExW(devKEY, L"EDID", NULL, &regType, byteBUFFER, &regSize);
            if (lResult != ERROR_SUCCESS) {
                std::cout << "ERROR!" << std::endl;
            }
            else
            {
                for (BYTE i = 0; i < 128; i++)
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << int(byteBUFFER[i]) << " ";
                std::cout << std::endl;
            }
            free(did_data);
            did_data = NULL;
            devFOUND = FALSE;
        }
        free(did_data);
    }

    // PREVENT memory leak and dangling pointer.

    //    delete[] ptrGUID;
    //    ptrGUID = nullptr;

    return true;

}

std::wstring PrimaryMonitor::GetHKEY(HKEY key)
{

    // FROM https://github.com/GKO95/Win32.EDID/blob/master/EDID.cpp/main.cpp
    std::wstring keyPath;
    if (key != NULL)
    {
        /*
            WINDOWS IS A family of OS developed by Microsoft; Windows NT is one of the active OS, which includes
            Windows 10, 8.1, 8, 7, Vista, XP, and 2000. And "ntdll.dll" module contains NT kernel functions.

            LoadLibraryW() function is responsible for loading modules such as DLL or EXE to the current process.
            The function can be deemed as counterpart of "modprobe" command in UNIX/Linux operating system.
        */
        HMODULE dll = LoadLibraryW(L"ntdll.dll");
        if (dll != NULL) {
            /*
                THE FUNCTION NEEDED for acquiring the registry key path is called "NtQueryKey()", but no header
                is available unless installing Windows Driver Kit. Thus, this is an attempt to declare the function
                without a help from "wdm.h" header file, identifying its function structure as "NtQueryKeyType".

                __stdcall is a keyword to specify it is the function directly called from the Win32 API.
            */
            typedef NTSTATUS(__stdcall* NtQueryKeyType)(
                        HANDLE  KeyHandle,
                        int KeyInformationClass,
                        PVOID  KeyInformation,
                        ULONG  Length,
                        PULONG  ResultLength);

            /*
                ACQRUIRING THE FUNCTION "NtQueryKey()" from the ntdll.dll module ("ZwQueryKey()" in kernel-mode)
                using "GerProcAddress()" function. The acquired function is returned as a function pointer.
            */
            NtQueryKeyType func = reinterpret_cast<NtQueryKeyType>(::GetProcAddress(dll, "NtQueryKey"));

            if (func != NULL) {
                /*
                    SUPPLYING THE FUNCTION "KEY_NAME_INFORMATION" (enumerated as integer 3) structure of the
                    registry key. However, since the buffer size is smaller, required size of the buffer will
                    be assigned to "size" variable, while returning STATUS_BUFFER_TOO_SMALL.
                */
                DWORD size = 0;
                DWORD result = 0;
                result = func(key, 3, 0, 0, &size);
                if (result == ((NTSTATUS)0xC0000023L))
                {
                    // Additional 2-byte for extra space when trimming first two insignificant bytes.
                    size = size + 2;
                    wchar_t* buffer = new (std::nothrow) wchar_t[size / sizeof(wchar_t)];
                    if (buffer != NULL)
                    {
                        result = func(key, 3, buffer, size, &size);
                        if (result == ((NTSTATUS)0x00000000L))
                        {
                            buffer[size / sizeof(wchar_t)] = L'\0';
                            keyPath = std::wstring(buffer + 2);
                        }

                        delete[] buffer;
                    }
                }
            }

            FreeLibrary(dll);
        }
    }

    return keyPath;
}

int PrimaryMonitor::getPosx() const
{
    return posx;
}

int PrimaryMonitor::getPosy() const
{
    return posy;
}

int PrimaryMonitor::getWidth() const
{
    return width;
}

int PrimaryMonitor::getHeight() const
{
    return height;
}

DWORD PrimaryMonitor::getLastBigthness() const
{
    return lastBigthness;
}

bool PrimaryMonitor::GetMonitorCap()
{
    BOOL bSuccess;
    int tries=0;
    DWORD lastError;
    do{
        bSuccess = GetMonitorCapabilities(pmonitor.hPhysicalMonitor, &dwMonitorCapabilities, &dwSupportedColorTemperatures);
        lastError = GetLastError();
        ++tries;
    }while(bSuccess==FALSE && (lastError == CheckSum_Error || lastError == InvalidDDCIMessage || lastError == InvalidDDCIValue || lastError == InvalidMonitorCapString));
    // 3223725451 - "An error occurred because the checksum field in a DDC/CI message did not match the message's computed checksum value. This error implies that the data was corrupted while it was being transmitted from a monitor to a computer.\r\n"

    if(!bSuccess){
        std::wcout << "GetMonitorCap ERROR " <<  GetLastError() << std::endl;
        throw WindowsErrorException(GetLastError());
    }

    return bSuccess;
}

