#ifndef __single_instance_win32_HPP__
#define __single_instance_win32_HPP__


#include <windows.h> 


namespace amorphous
{

/// This code is from Q243953 in case you lose the article and wonder
/// where this code came from.
class CLimitSingleInstance
{
protected:

	DWORD  m_dwLastError;
	HANDLE m_hMutex;

public:

	CLimitSingleInstance(const TCHAR *strMutexName)
	{
		// Make sure that you use a name that is unique for this application otherwise
		// two apps may think they are the same if they are using same name for
		// 3rd parm to CreateMutex
		m_hMutex = CreateMutex(NULL, FALSE, strMutexName); // do early
		m_dwLastError = GetLastError(); // save for use later...
	}

	~CLimitSingleInstance()
	{
		if (m_hMutex)  // Do not forget to close handles.
		{
			CloseHandle(m_hMutex); // Do as late as possible.
			m_hMutex = NULL; // Good habit to be in.
		}
	}

	bool IsAnotherInstanceRunning()
	{
		return (ERROR_ALREADY_EXISTS == m_dwLastError);
	}
};


inline bool is_another_instance_running( const char *unique_id )
{
	static CLimitSingleInstance s_obj( unique_id );
	return s_obj.IsAnotherInstanceRunning();
}

} // amorphous



#endif /* __single_instance_win32_HPP__ */
