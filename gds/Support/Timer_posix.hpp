#ifndef __Timer_posix_HPP__
#define __Timer_posix_HPP__


#include <sys/time.h>
#include "time.hpp"


/**
 Performs timer operations.
 - Use GetGlobalTimer() or macro 'TIMER' to get the global instance.
 */
class CTimer
{
public:

	CTimer();

	void Reset(); ///< resets the timer

	void Start(); ///< starts the timer

	void Stop();  ///< stop (or pause) the timer

	double GetAbsoluteTime(); ///< get the absolute system time [(in sec?)]

	double GetTime() const; ///< get the current time [s]

//	double GetFrameTime64(); // get the time that elapsed between GetElapsedTime() calls

	/// get the current time [ms]
	inline unsigned long GetTimeMS() const { return (unsigned long)(GetTime() * 1000.0); }

	/// returns the currenet frametime
	inline float GetFrameTime() const { return m_fFrameTime; }

	inline double GetFrameTime64() { return m_dFrameTime; }

	/// returns the frames per second
	inline float GetFPS() const { return m_fFPS; }

	/// updates the timer. must be called once per frame
	void UpdateFrameTime();

	bool IsStopped(); ///< returns true if timer stopped

//	void Advance(); // advance the timer by 0.1 seconds

protected:

	/// for PerformanceQueryCounter() version of the timer
	bool m_bTimerStopped;
/*	LONGLONG m_llQPFTicksPerSec;

	LONGLONG m_llStopTime;
	LONGLONG m_llLastElapsedTime;
	LONGLONG m_llBaseTime;

	/// time of the last FPS update
	LONGLONG m_llPrevTime;


	/// for timeGetTime() version of the timer, which will be used
	/// when the QueryPerformanceCounter() is not available

	/// time of the last UpdateFrameTime() call
    DWORD m_dwLastTimeMS;

	/// time of the last FPS update in UpdateFrameTime()
    DWORD m_dwPrevTimeMS;
*/
	int m_count;

	/// stores frametime. updated every frame in UpdateFrameTime()
	float m_fFrameTime;
	double m_dFrameTime;

	/// stores FPS. updated about every one second in UpdateFrameTime()
	float m_fFPS;

	struct timeval m_StartTime;

	struct timeval m_LastElapsedTime;
};


inline CTimer& GlobalTimer()
{
	// Using an accessor function gives control of the construction order
	// - This is a non-thread safe implementation of singleton pattern.
	// - Make sure GlobalTimer() is not called by more than one thread at the same time.
	static CTimer timer;

	static int initialized = 0;
	if( initialized == 0 )
	{
		timer.Start();
		initialized = 1;
	}

	return timer;
}


#endif  /* __Timer_posix_HPP__ */