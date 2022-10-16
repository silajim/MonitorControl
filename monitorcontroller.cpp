#include "monitorcontroller.h"
#include <iostream>
#include <Windows.h>
#include <QDebug>


MonitorController::MonitorController(QObject *parent)
    : QThread{parent}
{

}

MonitorController::~MonitorController()
{
    if(hwnd !=NULL){
        DestroyWindow(hwnd);
    }
    UnregisterDeviceNotification(devNotify);
}

void MonitorController::run()
{
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";

    WNDCLASSEX  wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = &MonitorController::WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;


    _atom = RegisterClassEx(&wc);
    if(_atom==NULL){
        std::wcout << "Register class " << GetLastError() << std::endl;
    }


    hwnd = CreateWindowEx(
                0,                              // Optional window styles.
                MAKEINTATOM(_atom),                     // Window class
                L"Learn to Program Windows",    // Window text
                WS_OVERLAPPEDWINDOW,            // Window style

                // Size and position
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                NULL,       // Parent window
                NULL,       // Menu
                GetModuleHandle(NULL),  // Instance handle
                NULL        // Additional application data
                );


    if(hwnd)
        std::cout << "HWND NOT NULL" << std::endl;
    else
        std::cout << "HWND NULL" << std::endl;

    SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)this);

    std::wcout << "Error " << GetLastError() << std::endl;

    GUID GUID_DEVINTERFACE_MONITOR = { 0xe6f07b5f, 0xee97, 0x4a90, 0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7 };

    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
    memset(&notificationFilter, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_reserved = 0;
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notificationFilter.dbcc_classguid = GUID_DEVINTERFACE_MONITOR;

    devNotify = RegisterDeviceNotification(hwnd, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);

    ShowWindow(hwnd, SW_HIDE);

    exec();
}


LRESULT CALLBACK MonitorController::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     std::cout << "WNDPROC " << uMsg << std::endl;
     MonitorController* mc = reinterpret_cast<MonitorController*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
     if (mc)
         return mc->MyWindowProc(hwnd, uMsg, wParam, lParam);
     else
         return DefWindowProc(hwnd, uMsg, wParam, lParam);


}

LRESULT MonitorController::MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    qDebug() << "MyWindowProc";

    switch (uMsg)
    {
    case WM_DEVICECHANGE:
    case WM_DISPLAYCHANGE:
        std::cout << "DISPLAY CHANGE" << std::endl;
        emit DisplaysChanged();
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

