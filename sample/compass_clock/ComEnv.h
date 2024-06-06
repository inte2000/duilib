#ifndef __COM_ENV_INIT_H__
#define __COM_ENV_INIT_H__

#define NOMINMAX
#include <windows.h>

class ComEnv final
{
public:
    ComEnv()
    {
        m_isInit = !FAILED(::CoInitialize(NULL));
    }
    ~ComEnv()
    {
        Release();
    }
    void Release()
    {
        if (m_isInit)
        {
            ::CoUninitialize();
            m_isInit = false;
        }
    }
    ComEnv (const ComEnv &) = delete;
    ComEnv(ComEnv &&) = delete;
    ComEnv & operator = (const ComEnv &) = delete;
    ComEnv& operator = (ComEnv &&) = delete;
private:
    bool m_isInit;
};

class ComEnvEx final
{
public:
    ComEnvEx(DWORD apartment)
    {
        m_isInit = (::CoInitializeEx(NULL, apartment) == S_OK) ? true : false;
    }
    ~ComEnvEx()
    {
        Release();
    }
    void Release()
    {
        if (m_isInit)
        {
            ::CoUninitialize();
            m_isInit = false;
        }
    }

    ComEnvEx (const ComEnvEx &) = delete;
    ComEnvEx(ComEnvEx &&) = delete;
    ComEnvEx & operator = (const ComEnvEx &) = delete;
    ComEnvEx& operator = (ComEnvEx &&) = delete;
private:
    bool m_isInit;
};

#endif //ifndef __COM_ENV_INIT_H__
