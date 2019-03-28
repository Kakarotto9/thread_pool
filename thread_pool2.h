#pragma once

#include"threadsafe_queue.h"
#include"function_wrapper.h"
#include"join_threads.h"
#include<atomic>
#include<vector>
#include<thread>
#include<future>

class thread_pool
{
	std::atomic_bool done;
	threadsafe_queue<function_wrapper> work_queue; // 使用function_wrapper，而非使用std:function
	std::vector<std::thread> threads;
	join_threads joiner;
		void worker_thread()
		{
			while(!done)
			{
				function_wrapper task;
				if(work_queue.try_pop(task))
				{
					task();

				}
				else
				{
					std::this_thread::yield();

				}

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
			done=true; // 10
			throw;
		}
		
	}
	~thread_pool(){
		done =true;
	}
	template<typename FunctionType>
		std::future<typename std::result_of<FunctionType()>::type> // 1
		submit(FunctionType f)
		{
			typedef typename std::result_of<FunctionType()>::type
				result_type; // 2
			std::packaged_task<result_type()> task(std::move(f)); // 3
			std::future<result_type> res(task.get_future()); // 4
			work_queue.push(std::move(task)); // 5
			return res; // 6

		}
	// 休息一下
	// 
};

