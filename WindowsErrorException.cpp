#include "WindowsErrorException.h"

WindowsErrorException::WindowsErrorException(DWORD errorCode) : m_errorCode(errorCode)
{

}

DWORD WindowsErrorException::errorCode() const
{
    return m_errorCode;
}
