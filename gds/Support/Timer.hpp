#ifndef __Timer_HPP__
#define __Timer_HPP__


#ifdef _MSC_VER
#include "Timer_Win32.hpp"
#else
#include "Timer_posix.hpp"
#endif


namespace amorphous
{


inline Timer& GlobalTimer()
{
	// Using an accessor function gives control of the construction order
	// - This is a non-thread safe implementation of singleton pattern.
	// - Make sure GlobalTimer() is not called by more than one thread at the same time.
	static Timer timer;

	static int initialized = 0;
	if( initialized == 0 )
	{
		timer.Start();
		initialized = 1;
	}

	return timer;
}

} // namespace amorphous



#endif /* __Timer_HPP__ */
