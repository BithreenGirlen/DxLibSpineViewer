#ifndef WIN_CLOCK_H_
#define WIN_CLOCK_H_

#include <Windows.h>

class CWinClock
{
public:
    CWinClock();
    ~CWinClock();

    float GetElapsedTime();
    void Restart();
private:
    LARGE_INTEGER m_nLastCounter{};
    LARGE_INTEGER m_frequency{};

    LARGE_INTEGER GetNowCounter();
};

#endif // !WIN_CLOCK_H_
