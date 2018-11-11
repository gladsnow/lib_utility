#include<iostream>
#include "lib_utility.h"

class TestThreads : public utility::MultiThreads<TestThreads,4>
{
public:
	TestThreads() { thread_exit_flag_ = FALSE; test_data_ = 0; };
	~TestThreads() {};

	void ThreadWorkFunc(THREAD_PARAMETERS* work_para)
	{
		while (!thread_exit_flag_)
		{
			Sleep(1000);
			//std::cout << "id=" << work_para->thread_id << std::endl;
			comm_sem_.WaitForSemSignaled();
			comm_mutex_.LockObject();
			test_data_++;
			std::cout << "id=" << work_para->thread_id << ",test_data="<< test_data_ << std::endl;
			comm_mutex_.UnlockObject();
			Sleep(3000);
			
			comm_sem_.ReleaseSemObject();
			Sleep((work_para->thread_id +1)*100);
			
		}
	};
	void StopThread()
	{
		thread_exit_flag_ = TRUE;
	}
	void OnBeforeThreadExiting()
	{
		std::cout << "OnBeforeExiting()." << std::endl;
	};

private:
	bool thread_exit_flag_;
	int test_data_;
	utility::CommonMutex comm_mutex_;
	utility::CommonSemaphore comm_sem_;
};
//
//int main() {
//
//	//std::queue<int> qtest;
//	//std::cout << "queue:front=" << qtest.front() << std::endl;
//
//	//utility::CycleQueue<int> test_q;
//	//for (int i = 0; i < 1; i++)
//	//{
//	//	test_q.Push(i);
//	//}
//	//std::cout << "isEmpty=" << test_q.IsEmpty() << std::endl;
//	//std::cout << "front=" << test_q.Front() << ",back=" << test_q.Back() << ",size=" << test_q.GetSize() << std::endl;
//
//	///*for (int i = 0; i < 1; i++)
//	//{
//	//	test_q.Pop();
//	//}*/
//	//test_q.Clear();
//	//std::cout << "clear,size=" << test_q.GetSize() << std::endl;
//	utility::DoubleLinkedList<int> dlist;
//	for (int i = 0; i < 10; i++)
//	{
//		dlist.InsertNode(i);
//	}
//	std::cout << "size=" << dlist.GetSize() << std::endl;
//
//	dlist.PrintList();
//	std::cout << "Find result:" << dlist.FindNode(10) << std::endl;
//	std::cout << "Find result:" << dlist.FindNode(5) << std::endl;
//	dlist.DeleteNode(7);
//	std::cout << "delete,size=" << dlist.GetSize() << std::endl;
//	dlist.EmptyList();
//	std::cout << "Empty,size=" << dlist.GetSize() << std::endl;
//	system("pause");
//	return 0;
//}

#include <iostream>
#include <thread>
#include <chrono>
//#include "timer_wheel.h"
#include "lib_utility.h"

class OnNotify : public utility::TimerNotify
{
public:
	OnNotify(){}
	~OnNotify() {}

	void OnTimerNotify() { std::cout << "OnTimerNotify" << std::endl; }
};

void TimerHandler()
{
	std::cout << "TimerHandler" << std::endl;
}

int main()
{
	utility::TimerManager tm;
	utility::Timer t(tm);
	utility::TimerNotify* timer_notify = new OnNotify();
	t.StartTimer(timer_notify, 3000);

	while (true)
	{
		//unsigned long long start = time.GetCurrentMilliseconds();
		//unsigned long long start = tm.GetCurrentMillisecs();
		tm.DetectTimers();
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		//unsigned long long end = time.GetCurrentMilliseconds();
		//unsigned long long end = tm.GetCurrentMillisecs();
		//std::cout << "time:" << end - start << std::endl;
	}

	std::cin.get();
	return 0;
}