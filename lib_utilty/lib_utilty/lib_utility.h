#ifndef _LIB_UTILITY_H_
#define _LIB_UTILITY_H_

#ifdef _WIN32
#include <windows.h>
#include <string>
#include<process.h>
#include <assert.h>
#include <vector>
#include <list>
#include <map>
#endif

namespace utility {

class CommonMutex
{
public:
	CommonMutex(std::string mutex_name = "");
	~CommonMutex();

	bool LockObject(void);
	bool UnlockObject(void);

private:
#ifdef _WIN32
/*CRITICAL_SECTIONֻ�����ڶ��߳�֮��.
�����Ҫ�ڶ������ʹ�û�������
���ڱ����ʱ��Ӻ궨��MULTIPROCESS
*/
#ifndef MULTIPROCESS
	CRITICAL_SECTION thread_lock_;
#else
	std::string mutex_name_;
	HANDLE mutex_handle_;
#endif
#endif // _WIN32
};

class CommonEvent
{
public:
	CommonEvent(std::string event_name = "");
	~CommonEvent();
	bool WaitForEventSignaled(int nMillonSecond = 0);
	bool SetEvent();
	bool ResetEvent();

private:
	std::string event_name_;
#ifdef _WIN32
	HANDLE  event_handle_;
#endif // _WIN32

};

class CommonSemaphore
{
public:
	CommonSemaphore(LONG init_sem_count = 2, LONG max_sem_count = 3,std::string semaphore_name = "");
	~CommonSemaphore();
	bool WaitForSemSignaled(int nMillonSecond = 0);
	bool ReleaseSemObject();

private:
	std::string semaphore_name_;
	LONG init_sem_count_;
	LONG max_sem_count_;

#ifdef _WIN32
	HANDLE semaphore_handle_;
#endif // _WIN32

};

template <class T, int DEFAULT_THREAD_COUNT = 10>
class MultiThreads
{
public:
	MultiThreads()
	{
		thread_count_ = 0;
		thread_handle_ = NULL;
		thread_params_ = NULL;
	};
	virtual ~MultiThreads()
	{
		DestroyThreads();
	};

	typedef struct {
		int	thread_id;
		T* thread_class_ptr;
		int result;
	}THREAD_PARAMETERS;

	virtual void ThreadWorkFunc(THREAD_PARAMETERS* work_para) = 0;
	virtual void OnBeforeThreadExiting() {};

	BOOL CreateThread(int thread_count = DEFAULT_THREAD_COUNT)
	{
		thread_count_ = thread_count;
		thread_handle_ = new HANDLE[thread_count_];
		thread_params_ = new THREAD_PARAMETERS[thread_count_];

		if (thread_handle_ == NULL || thread_params_ == NULL)
		{
			return FALSE;
		}

		for (int i = 0; i < thread_count_; i++)
		{
			thread_params_[i].thread_class_ptr = (T*)this;
			thread_params_[i].result = 0;
			thread_params_[i].thread_id = i;

			thread_handle_[i] = (HANDLE*)_beginthreadex(NULL,
				0,
				(unsigned int(__stdcall *)(void *))ThreadProcessFunc,
				(PVOID)&thread_params_[i],
				0,
				NULL);

			if (thread_handle_[i] == 0)
			{
				return FALSE;
			}
		}

		return TRUE;
	};

	static void ThreadProcessFunc(void* param)
	{
		THREAD_PARAMETERS* thread_param = (THREAD_PARAMETERS*)param;
		MultiThreads* thread_ptr = thread_param->thread_class_ptr;
		if (thread_ptr != NULL)
		{
			thread_ptr->ThreadWorkFunc(thread_param);
		}
	}

	void DestroyThreads()
	{
		OnBeforeThreadExiting();

		if (thread_handle_ == NULL && thread_params_ == NULL)
		{
			return;
		}
		WaitThreadsExit();

		for (int i = 0; i < thread_count_; i++)
		{
#ifdef _WIN32
			CloseHandle(thread_handle_[i]);
#endif
			if (thread_handle_ != NULL)
			{
				delete[] thread_handle_;
				thread_handle_ = NULL;
			}
			if (thread_params_ != NULL)
			{
				delete[] thread_params_;
				thread_params_ = NULL;
			}

			thread_count_ = 0;
		}
	}

private:
	void WaitThreadsExit()
	{
		/*�ȴ��߳��˳���ʱ����5s,��5s֮���̻߳�δ�˳������ǿ���߳��˳�*/
		WaitForMultipleObjects(thread_count_, thread_handle_, TRUE, 5000);
	}

private:
	int thread_count_;
	HANDLE* thread_handle_;
	THREAD_PARAMETERS* thread_params_;
};

class SystemTime
{
public:
	SystemTime();
	~SystemTime();
	unsigned long long GetCurrentMilliseconds();
private:
	DOUBLE frequency_;
};

/***********************************************
���ڷֲ�ʱ���ֵĶ�ʱ��
*�ܹ���Ϊ5������һ��ʱ���ֵĴ�С2^8��
*�����ĸ��ֵĴ�С�ֱ�Ϊ2^6��
*���ʱ���ֵ���С�̶�ΪTms,��ʱ���ֵĶ�ʱ��ΧΪ
*2^8 * 2^6 * 2^6 * 2^6 * 2^6 * T = 2^32 * T ms
*��0~2^32 * T,����ΪTms.
************************************************/

#define WHEEL_SCALE 500 //��һ��ʱ���ֵ�һ����500ms
#define WHEEL_BIT1	8
#define WHEEL_BIT2	6
#define WHEEL_SIZE1 (1 << WHEEL_BIT1)	//��1��ʱ���ֵĴ�С
#define WHEEL_SIZE2 (1 << WHEEL_BIT2)	//��2~5��ʱ���ֵĴ�С
#define WHEEL_MASK1 (WHEEL_SIZE1 - 1)
#define WHEEL_MASK2 (WHEEL_SIZE2 - 1)
#define OFFSET(N) (WHEEL_SIZE1 + (N) * WHEEL_SIZE2)
#define INDEX(N,M) ((N >> (WHEEL_BIT1 + (M)*WHEEL_BIT2))& WHEEL_MASK2)


//��ʱ������֪ͨ
class TimerNotify
{
public:
	TimerNotify() {};
	virtual ~TimerNotify() {};
	virtual void OnTimerNotify() = 0;

};

enum TimerType { ONCE, CIRCLE };

//��ʱ������
class TimerTask
{
public:

	TimerTask();
	~TimerTask();
	void SetTimerTask(TimerNotify* timer_notify, unsigned interval, TimerType timer_type = CIRCLE);

	unsigned GetIntervalTime();
	void SetVectorIndex(int vect_index);
	int GetVectorIndex(void);
	void HandleTask();

	std::list<TimerTask*>::iterator itr_;
private:
	unsigned interval_time_;
	int vect_index_;
	TimerNotify* timer_notify_;
	TimerType timer_type_;

};

//����ʱ��
class TimerManager
{
public:
	TimerManager();
	~TimerManager();

	void AddTimer(TimerTask* timer);
	void RemoveTimer(TimerTask* timer);
	int Cascade(int offset, int index);
	void DetectTimers(void);
private:
	typedef std::list<TimerTask*> TIMER_LIST;
	std::vector<TIMER_LIST> timer_wheel_;
	unsigned current_pos_;
};

//��ʱ���߳�
class TimerThread: public utility::MultiThreads<TimerThread,1>
{
public:
	TimerThread();
	~TimerThread();
	 
	void ThreadWorkFunc(THREAD_PARAMETERS* work_para);
	void OnBeforeThreadExiting();

	BOOL StartTimerThread();//������ʱ���߳�
	void StopTimerThread();//ֹͣ��ʱ���߳�

	//����һ����ʱ������
	TimerTask* SetATimer(TimerNotify* timer_notify, unsigned interval_time, TimerType timeType = CIRCLE);
	void StopATimer(TimerTask* timer_task);//ֹͣһ����ʱ������
	
private:
	TimerManager timer_manager_;
	std::list<TimerTask*> task_list_;
	BOOL exit_flag_;
	utility::CommonEvent comm_event_;
};

}
#endif //_LIB_UTILITY_H_