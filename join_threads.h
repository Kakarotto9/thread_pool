#pragma once

#include<thread>
#include<vector>


//vector<thread>资源管理类
class join_threads
{
	std::vector<std::thread>& threads;  //必须是&
	public:
	explicit join_threads(std::vector<std::thread>& threads_):
		threads(threads_)
	{}
	~join_threads()
	{
		for(unsigned long i=0;i<threads.size();++i)
		{
			if(threads[i].joinable())
				threads[i].join();
			
		}
		
	}
	
};
