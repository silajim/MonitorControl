#ifndef MONITORCONTROLLER_H
#define MONITORCONTROLLER_H

#include <Windows.h>
#include <QThread>
#include <QObject>
#include <Dbt.h>

class MonitorController : public QThread
{
    Q_OBJECT
public:
    explicit MonitorController(QObject *parent = nullptr);
    ~MonitorController();
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

signals:
    void DisplaysChanged();

private:
    HWND hwnd = NULL;
    ATOM _atom = NULL;
    HDEVNOTIFY devNotify=NULL;
    LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



    // QThread interface
protected:
    void run() override;
};

#endif // MONITORCONTROLLER_H
