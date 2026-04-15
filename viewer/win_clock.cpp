

#include "win_clock.h"

CWinClock::CWinClock()
{
	::QueryPerformanceFrequency(&m_frequency);
	restart();
}

CWinClock::~CWinClock()
{

}

float CWinClock::getElapsedTime()
{
	LARGE_INTEGER nNow = getTicks();
	return static_cast<float>(nNow.QuadPart - m_nLastCount.QuadPart) / m_frequency.QuadPart;
}

void CWinClock::restart()
{
	m_nLastCount = getTicks();
}

LARGE_INTEGER CWinClock::getTicks()
{
	LARGE_INTEGER ticks;
	::QueryPerformanceCounter(&ticks);
	return ticks;
}
