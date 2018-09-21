#pragma once
#include "SingletonBase.h"




//***************************************************************************************
// TimeMGR.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************
// 

class TimeMGR : public SingletonBase<TimeMGR>
{
public:
	TimeMGR();
	~TimeMGR();

	float TotalTime() const;
	float DeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double m_SecondsPerCount;
	double m_DeltaTime;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurTime;

	bool m_Stopped;



};

