#pragma once

#include<atomic>
#include<thread>
#include<vector>
#include<functional>
#include"threadsafe_queue.h"
#include"join_threads.h"

class thread_pool
{
	/*
	 注意成员变量声明的顺序，这是为了保证在析构时能正确析构,所有成员必须在joiner前声
	 明(done标志和worker_queue必须在threads数组之前声明);
	 */
	std::atomic_bool done;
	threadsafe_queue<std::function<void()> > work_queue; // 1
	std::vector<std::thread> threads; // 2
	join_threads joiner; // 3
	void worker_thread()
	{
		while(!done) // 4
		{
			std::function<void()> task;
			work_queue.wait_pop(task); // 5
			task(); // 6
		}
		
	}
	public:
	thread_pool():
		done(false),joiner(threads)
	{
		unsigned const thread_count=std::thread::hardware_concurrency(); // 8
		try
		{
			for(unsigned i=0;i<thread_count;++i)
			{
				threads.push_back(
						std::thread(&thread_pool::worker_thread,this)
						); // 9
				
			}
			
		}
		catch(...)
		{

			done=true; //保证work_thread可以结束循环 
			throw;
		}
		
	}
	~thread_pool()
	{
		done=true; // 11
		
	}
	template<typename FunctionType>
	void submit(FunctionType f)
	{
		work_queue.push(std::function<void()>(f)); // 12
	}
};
