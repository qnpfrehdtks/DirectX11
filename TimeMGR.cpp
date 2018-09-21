#include "DXUT.h"
#include <windows.h>
#include "TimeMGR.h"

//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************


TimeMGR::TimeMGR()
	: m_SecondsPerCount(0.0), m_DeltaTime(-1.0), m_BaseTime(0),
	m_PausedTime(0), m_PrevTime(0), m_CurTime(0), m_Stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;
}


TimeMGR::~TimeMGR()
{
}

float TimeMGR::TotalTime() const
{
	if (m_Stopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime)* m_SecondsPerCount);
	}
	else
	{
		return (float)(((m_CurTime - m_PausedTime) - m_BaseTime)* m_SecondsPerCount);
	}
}

float TimeMGR::DeltaTime() const
{
	return m_DeltaTime;
}

void TimeMGR::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_Stopped = false;

}

void TimeMGR::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (m_Stopped)
	{
		m_PausedTime += (startTime - m_StopTime);

		m_PrevTime = startTime;
		m_StopTime = 0;
		m_Stopped = false;
	}
}

void TimeMGR::Stop()
{
	if (!m_Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_StopTime = currTime;
		m_Stopped = true;
	}
}

void TimeMGR::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurTime = currTime;

	// Time difference between this frame and the previous.
	m_DeltaTime = (m_CurTime - m_PrevTime)* m_SecondsPerCount;

	// Prepare for next frame.
	m_PrevTime = m_CurTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0;
	}
}
