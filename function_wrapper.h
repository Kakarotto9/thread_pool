#pragma once

#include<memory>

class function_wrapper
{
	struct impl_base {
		//不设置纯虚函数也可以
		virtual void call()=0;
		virtual ~impl_base() {}

	};
	template<typename F>
	struct impl_type: impl_base
	{
		F f;
		impl_type(F&& f_): f(std::move(f_)) {}
		void call() { f();  }

	};
	//使用impl_base*也可以，不过不建议
	std::unique_ptr<impl_base> impl;

	public:
	function_wrapper() = default;
	// f虽然是右值引用，但f本身是左值，需要使用std::move()转换
	template<typename F>
	function_wrapper(F&& f):impl(new impl_type<F>(std::move(f)))
	{}
	function_wrapper(function_wrapper&& other):impl(std::move(other.impl))
	{}
	function_wrapper& operator=(function_wrapper&& other)
	{
		impl=std::move(other.impl);
		return *this;
	}
	function_wrapper(const function_wrapper&)=delete;
	function_wrapper(function_wrapper&)=delete;
	function_wrapper& operator=(const function_wrapper&)=delete;

	void operator()()
   	{
	   	impl->call();  
	}
};

