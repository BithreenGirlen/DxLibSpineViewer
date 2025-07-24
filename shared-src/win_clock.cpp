

#include "win_clock.h"

CWinClock::CWinClock()
{
	::QueryPerformanceFrequency(&m_frequency);
	Restart();
}

CWinClock::~CWinClock()
{

}

float CWinClock::GetElapsedTime()
{
	LARGE_INTEGER nNow = GetNowCounter();
	return static_cast<float>(nNow.QuadPart - m_nLastCounter.QuadPart) / m_frequency.QuadPart;
}

void CWinClock::Restart()
{
	m_nLastCounter = GetNowCounter();
}

LARGE_INTEGER CWinClock::GetNowCounter()
{
	LARGE_INTEGER ticks;
	::QueryPerformanceCounter(&ticks);
	return ticks;
}
