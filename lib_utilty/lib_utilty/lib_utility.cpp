#include "lib_utility.h"
#include <chrono>
namespace utility {
	CommonMutex::CommonMutex(std::string mutex_name)
	{
#ifndef MULTIPROCESS
		InitializeCriticalSection(&thread_lock_);
#else
		mutex_name_ = mutex_name;
		mutex_handle_ = CreateMutex(NULL, FALSE, (mutex_name_ != "") ? mutex_name_.c_str() : NULL);
#endif
	}

	CommonMutex::~CommonMutex()
	{
#ifndef MULTIPROCESS
		DeleteCriticalSection(&thread_lock_);
#else
		CloseHandle(mutex_handle_);
#endif	
	}

	bool CommonMutex::LockObject(void)
	{
#ifndef MULTIPROCESS
		EnterCriticalSection(&thread_lock_);
#else
		if (NULL == mutex_handle_)
		{
			return FALSE;
		}
		DWORD nRet = WaitForSingleObject(mutex_handle_, INFINITE);
		if (nRet != WAIT_OBJECT_0)
		{
			return FALSE;
		}	
#endif
		return TRUE;
	}

	bool CommonMutex::UnlockObject(void)
	{
#ifndef MULTIPROCESS
		LeaveCriticalSection(&thread_lock_);
		return TRUE;
#else
		return ReleaseMutex(mutex_handle_);
#endif
	}

	CommonEvent::CommonEvent(std::string event_name)
	{
		event_name_ = event_name;
		event_handle_ = CreateEvent(NULL, TRUE, FALSE,(event_name_!="")?event_name_.c_str():NULL);
	}
	CommonEvent::~CommonEvent()
	{
		CloseHandle(event_handle_);
	}
	bool CommonEvent::WaitForEventSignaled(int nMillonSecond)
	{
		if (NULL == event_handle_)
		{
			return FALSE;
		}
		DWORD ret;
		if (nMillonSecond > 0)
		{
			ret = ::WaitForSingleObject(event_handle_, nMillonSecond);
		}
		else
		{
			ret = ::WaitForSingleObject(event_handle_, INFINITE);
		}
		if (ret != WAIT_OBJECT_0)
		{
			return FALSE;
		}

		return TRUE;
	}
	bool CommonEvent::SetEvent()
	{
		BOOL ret = false;
#ifdef _WIN32
		ret = ::SetEvent(event_handle_);
#endif // _WIN32

		return ret;
	}
	bool CommonEvent::ResetEvent()
	{
		BOOL ret = false;
#ifdef _WIN32
		ret = ::ResetEvent(event_handle_);
#endif // _WIN32
		return ret;
	}


	CommonSemaphore::CommonSemaphore(LONG init_sem_count, LONG max_sem_count, std::string semaphore_name)
	{
		if (init_sem_count < 0)
			init_sem_count_ = 1;
		else
			init_sem_count_ = init_sem_count;

		if (max_sem_count <= 0)
			max_sem_count_ = 1;
		else
			max_sem_count_ = max_sem_count;

		semaphore_name_ = semaphore_name;
		semaphore_handle_ = CreateSemaphore(NULL, init_sem_count, max_sem_count, semaphore_name_ != "" ? semaphore_name_.c_str():NULL);
	}
	CommonSemaphore::~CommonSemaphore()
	{
		CloseHandle(semaphore_handle_);
	}

	bool CommonSemaphore::WaitForSemSignaled(int nMillonSecond)
	{
		DWORD ret;
		if (NULL == semaphore_handle_)
		{
			return FALSE;
		}
		
		if (nMillonSecond > 0)
		{
			ret = ::WaitForSingleObject(semaphore_handle_, nMillonSecond);
		}
		else
		{
			ret = ::WaitForSingleObject(semaphore_handle_, INFINITE);
		}
		if (ret != WAIT_OBJECT_0)
		{
			return FALSE;
		}
		return TRUE;
	}
	bool CommonSemaphore::ReleaseSemObject()
	{
		return  ReleaseSemaphore(semaphore_handle_, 1, NULL);
	}


	SystemTime::SystemTime(void)
	{
		LARGE_INTEGER fc;

		if (!QueryPerformanceFrequency(&fc))
		{
			assert(FALSE);
		}
		frequency_ = (DOUBLE)fc.QuadPart;  //计时器的频率
	}

	SystemTime::~SystemTime(void)
	{
	}
	UINT64 SystemTime::GetCurrentMilliseconds()
	{
		assert(frequency_ > 1000);
		LARGE_INTEGER fc;
		if (!QueryPerformanceCounter(&fc))
		{
			assert(FALSE);
		}

		return UINT64((fc.QuadPart) / (frequency_ /1000));
	}

	Timer::Timer()
	{

	}
	Timer::~Timer()
	{

	}

	UINT32 Timer::GeIntervalTime()
	{
		return interval_time_;
	}

	TimerManager::TimerManager()
	{
		check_time_ = sys_time_.GetCurrentMilliseconds();
	}
	TimerManager::~TimerManager()
	{

	}

	BOOL TimerManager::AddTimer(Timer* timer)
	{
		UINT64 interval_time = timer->GeIntervalTime();
		UINT64 interval = interval_time / GRANULARITY;
		if (interval < WHEEL_SIZE1)
		{
			
		}


		return TRUE;
	}
	void TimerManager::RemoveTimer(int timer_id)
	{

	}


}
