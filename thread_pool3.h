#pragma once

#include"threadsafe_queue.h"
#include"join_threads.h"
#include"function_wrapper.h"
#include<atomic>
#include<thread>
#include<vector>
#include<queue>
#include<future>


class thread_pool
{
	std::atomic_bool done;
	std::vector<std::thread> threads;
	threadsafe_queue<function_wrapper> pool_work_queue;
	typedef std::queue<function_wrapper> local_queue_type; // 1
	static thread_local std::unique_ptr<local_queue_type>	local_work_queue; //  注意使用 static thread_locl,保证每个线程有自己的queue
	join_threads joiner;
	void worker_thread()
	{
		local_work_queue.reset(new local_queue_type); // 
		while(!done)
		{
			run_pending_task();

		}

	}
	public:
	thread_pool():	done(false),joiner(threads)
	{
		unsigned const thread_count=std::thread::hardware_concurrency(); 
		try
		{
			for(unsigned i=0;i<thread_count;++i)
			{
				threads.push_back(
						std::thread(&thread_pool::worker_thread,this)
						); 
			}
		}
		catch(...)
		{
			done=true; 
			throw;
		}

	}
	~thread_pool()
	{
		done=true; 
	}
	template<typename FunctionType>
		std::future<typename std::result_of<FunctionType()>::type>
		submit(FunctionType f)
		{
			typedef typename std::result_of<FunctionType()>::type result_type;
			std::packaged_task<result_type()> task(f);
			std::future<result_type> res(task.get_future());
			if(local_work_queue) //先调用本地的 
			{
				local_work_queue->push(std::move(task));
			}
			else
			{
				pool_work_queue.push(std::move(task)); //使用std::move
			}
			return res;

		}
	void run_pending_task()
	{
		function_wrapper task;
		if(local_work_queue && !local_work_queue->empty()) // 6
		{
			/*不要直接 task=std::move(local_work_queue->pop());
			 * 可能在返回值产生 内存不够的异常，从而导致pop中删除了，但没有传递给task
			 */
			task=std::move(local_work_queue->front());
			local_work_queue->pop();
			task();
		}
		else if(pool_work_queue.try_pop(task)) //这里不能使用wait_pop(); 
		{
			task();
		}
		else
		{
			std::this_thread::yield();
		}
	}
};
