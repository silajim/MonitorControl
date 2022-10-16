#ifndef ERROREXCEPTION_H
#define ERROREXCEPTION_H

#include <exception>

#include <Windows.h>

class WindowsErrorException : public std::exception
{
public:
    WindowsErrorException(DWORD errorCode);

    DWORD errorCode() const;

private:
    DWORD m_errorCode;
};

#endif // ERROREXCEPTION_H
