#include "lib_utility.h"
#include <chrono>
#include <iostream>
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

	unsigned long long SystemTime::GetCurrentMilliseconds()
	{
		assert(frequency_ > 1000);
		LARGE_INTEGER fc;
		if (!QueryPerformanceCounter(&fc))
		{
			assert(FALSE);
		}

		return unsigned long long((fc.QuadPart) / (frequency_ /1000));
	}

	TimerTask::TimerTask()
	{
		interval_time_ = 0;
		vect_index_ = -1;
		timer_notify_ = NULL;
		timer_type_ = CIRCLE;
	}

	TimerTask::~TimerTask()
	{
		if (timer_notify_ != NULL)
		{
			delete timer_notify_;
		}
	}

	void TimerTask::SetTimerTask(TimerNotify* timer_notify, unsigned interval, TimerType timer_type)
	{
		interval_time_ = interval;
		timer_notify_ = timer_notify;
		timer_type_ = timer_type;
	}

	unsigned TimerTask::GetIntervalTime()
	{
		return interval_time_;
	}
	void TimerTask::SetVectorIndex(int vect_index)
	{
		vect_index_ = vect_index;
	}

	int TimerTask::GetVectorIndex(void)
	{
		return vect_index_;
	}

	void TimerTask::HandleTask()
	{
		
		if (timer_type_ != CIRCLE)
		{
			vect_index_ = -1;
		}

		if (timer_notify_ != NULL)
		{
			timer_notify_->OnTimerNotify();
		}
	}

	TimerManager::TimerManager()
	{
		timer_wheel_.resize(WHEEL_SIZE1 + 4 * WHEEL_SIZE2);
		current_pos_ = 0;
	}

	TimerManager::~TimerManager()
	{

	}

	void TimerManager::AddTimer(TimerTask* timer_task)
	{
		unsigned long long interval_time = timer_task->GetIntervalTime();
		unsigned long long tmp_interval = interval_time /WHEEL_SCALE;
		int vect_index = 0;

		if (tmp_interval < WHEEL_SIZE1)
		{
			vect_index = (tmp_interval + current_pos_) % WHEEL_SIZE1;
		}
		else if (tmp_interval < ((unsigned)1 << (WHEEL_BIT1 + WHEEL_BIT2)))
		{
			vect_index = INDEX(tmp_interval + current_pos_, 0) % WHEEL_SIZE2 + OFFSET(0);
		}
		else if (tmp_interval < ((unsigned)1 << (WHEEL_BIT1 + 2 * WHEEL_BIT2)))
		{
			vect_index = INDEX(tmp_interval + current_pos_, 1) % WHEEL_SIZE2 + OFFSET(1);
		}
		else if (tmp_interval < ((unsigned)1 << (WHEEL_BIT1 + 3 * WHEEL_BIT2)))
		{
			vect_index = INDEX(tmp_interval + current_pos_, 2) % WHEEL_SIZE2 + OFFSET(2);
		}
		else if (tmp_interval < ((unsigned long long)1 << (WHEEL_BIT1 + 4 * WHEEL_BIT2)))
		{
			vect_index = INDEX(tmp_interval + current_pos_, 3) % WHEEL_SIZE2 + OFFSET(3);
		}

		timer_task->SetVectorIndex(vect_index);

		TIMER_LIST& tlist = timer_wheel_[timer_task->GetVectorIndex()];
		tlist.push_back(timer_task);
		timer_task->itr_ = tlist.end();
		--timer_task->itr_;
	}

	void TimerManager::RemoveTimer(TimerTask* timer_task)
	{
		TIMER_LIST& tlist = timer_wheel_[timer_task->GetVectorIndex()];
		tlist.erase(timer_task->itr_);
	}

	void TimerManager::DetectTimers()
	{
		if (current_pos_ < 0xffffffffUL)
		{
			int index = (current_pos_) % WHEEL_SIZE1;
			if (!index &&
				!Cascade(OFFSET(0), INDEX(current_pos_ , 0)) &&
				!Cascade(OFFSET(1), INDEX(current_pos_, 1)) &&
				!Cascade(OFFSET(2), INDEX(current_pos_, 2)))
			{
				Cascade(OFFSET(3), INDEX(current_pos_ , 3));
			}

			Sleep(WHEEL_SCALE);

			TIMER_LIST& tlist = timer_wheel_[index];
			TIMER_LIST temp;
			temp.splice(temp.end(), tlist);
			for (TIMER_LIST::iterator itr = temp.begin(); itr != temp.end(); ++itr)
			{
				(*itr)->HandleTask();
				if ((*itr)->GetVectorIndex() != -1)
				{
					AddTimer(*itr);
				}
			}
			current_pos_ += 1;
		}
		else
		{
			current_pos_ = 0;
		}
	}

	int TimerManager::Cascade(int offset, int index)
	{
		TIMER_LIST& tlist = timer_wheel_[offset + index];
		TIMER_LIST temp;
		temp.splice(temp.end(), tlist);

		for (TIMER_LIST::iterator itr = temp.begin(); itr != temp.end(); ++itr)
		{
			AddTimer(*itr);
		}

		return index;
	}

	TimerThread::TimerThread() 
	{
		exit_flag_ = FALSE;
		task_list_.clear();
	}

	TimerThread::~TimerThread()
	{
		task_list_.clear();
	}

	void TimerThread::ThreadWorkFunc(THREAD_PARAMETERS* work_para)
	{
		while (!exit_flag_)
		{
			if (task_list_.size() == 0)
			{
				comm_event_.ResetEvent();
				comm_event_.WaitForEventSignaled();
			}
			timer_manager_.DetectTimers();
		}
	}

	TimerTask* TimerThread::SetATimer(TimerNotify* timer_notify, unsigned interval_time, TimerType timeType)
	{
		if (interval_time >= 0xFFFFFFFFUL)
		{
			return NULL;
		}
		TimerTask* timer_task = new TimerTask();
		if (timer_task == NULL)
		{
			return NULL;
		}
		timer_task->SetTimerTask(timer_notify, interval_time, timeType);
		timer_manager_.AddTimer(timer_task);
		task_list_.push_back(timer_task);
		comm_event_.SetEvent();
		return timer_task;
	}

	void TimerThread::StopATimer(TimerTask* timer_task)
	{
		timer_manager_.RemoveTimer(timer_task);
		task_list_.remove(timer_task);

		if (timer_task != NULL)
		{
			delete timer_task;
		}
	}

	BOOL TimerThread::StartTimerThread()
	{
		return CreateThread();
	}
	
	void TimerThread::StopTimerThread()
	{
		exit_flag_ = TRUE;
		DestroyThreads();
	}
	void TimerThread::OnBeforeThreadExiting()
	{
		while (task_list_.size() > 0)
		{
			TimerTask* tmp_task = task_list_.front();
			StopATimer(tmp_task);
		}
	}
}
