// -----------------------------------------------------------------------------
// File name  : thread_pool_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-20
// Description: thread_pool test file
// -----------------------------------------------------------------------------
#include <iostream>
#include <functional>
#include <unistd.h> //sleep

#include "scope_lock.hpp"
#include "thread_pool.hpp"

using namespace hrd8;

class test_task : public ThreadPool::Task
{
public:
	test_task(Priority priority = Priority::MEDIUM): Task(priority) {}
	
	static size_t m_counter;
private:
	static std::mutex m_mut;
	void execute() noexcept override;
};

size_t test_task::m_counter = 0;
std::mutex test_task::m_mut;

void test_task::execute() noexcept
{
	ScopeLock<std::mutex> lock(m_mut);
	++m_counter;
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	// std::cout << "task number: " << m_counter << std::endl; // debug
}

class test_task_2 : public ThreadPool::Task
{
public:
	test_task_2(Priority priority, const std::string& str):
	Task(priority), m_str(str) {}
	
	static size_t m_counter;
private:
	std::string m_str;
	static std::mutex m_mut_2;
	void execute() noexcept override
	{
		ScopeLock<std::mutex> l(m_mut_2);
		// std::cout << "task: my priority is: " << static_cast<int>(m_priority) 
		// 	<< std::endl;
		std::cout << m_str;
	}
};
std::mutex test_task_2::m_mut_2;

//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n============= thread_pool test ==============\n" << std::endl;
	{
		std::shared_ptr<test_task> p1(new test_task());
		
		size_t num_of_threads = 7;
		
		// create thread pool
		ThreadPool tp(num_of_threads, std::chrono::milliseconds(700));
		
		// add_task + suspend + resume
		size_t num_of_tasks = 400;
		for (size_t i = 0; i < num_of_tasks; i++)
		{
			tp.add_task(p1);
		}
		
		std::cout << "suspend: wait 1 second?\t";
		tp.suspend();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "SUCCESS" << std::endl;
		
		std::cout << "resume:\t\t\t";
		tp.resume();
		std::this_thread::sleep_for(std::chrono::milliseconds(700)); // make sure all tasks are done
		(test_task::m_counter == num_of_tasks)
		?
		std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
		
		// resize up test
		std::cout << "resize up:\t\tSUCCESS" << std::endl;
		tp.resize(num_of_threads + 10);
		
		// stop (resize(0)) test
		std::cout << "stop (resize(0)):\t";
		test_task::m_counter = 0;
		for (size_t i = 0; i < num_of_tasks; i++)
		{
			tp.add_task(p1);
		}
		tp.stop();
		
		(test_task::m_counter < num_of_tasks)
		?
		std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	}
	
	//////////////////////////////////////////////////////////////
	
	std::cout << "\n---------------- task test -------------------\n" << std::endl;
	{
		std::shared_ptr<test_task_2> p1(new test_task_2
			(ThreadPool::Task::Priority::MEDIUM, std::string("CC")));
		std::shared_ptr<test_task_2> p2(new test_task_2
			(ThreadPool::Task::Priority::LOW, std::string("ESS")));
		std::shared_ptr<test_task_2> p3(new test_task_2
			(ThreadPool::Task::Priority::HIGH, std::string("SU")));
		
		// priority test
		std::cout << "priority test:\t\t";
		ThreadPool tp(1, std::chrono::milliseconds(3000));
		
		tp.suspend();
		
		tp.add_task(p1);
		tp.add_task(p2);
		tp.add_task(p3);
		
		tp.resume();
		// should print "SUCCESS" correctly
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// tp.stop();
	}	
	std::cout << "\n===========================================\n" << std::endl;
	
	return (0);
}
