#ifndef WIN_CLOCK_H_
#define WIN_CLOCK_H_

#include <Windows.h>

class CWinClock
{
public:
    CWinClock();
    ~CWinClock();

    float getElapsedTime();
    void restart();
private:
    LARGE_INTEGER m_nLastCount{};
    LARGE_INTEGER m_frequency{};

    LARGE_INTEGER getTicks();
};

#endif // !WIN_CLOCK_H_
