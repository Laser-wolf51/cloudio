// -----------------------------------------------------------------------------
// File name  : thread_pool.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-20
// Description: thread_pool source file
// -----------------------------------------------------------------------------
#include <cmath> //abs
#include <iostream> //cerr

#include "../../utils/scope_lock.hpp"
#include "thread_pool.hpp"

namespace hrd8
{

//========================== Task API ========================================//
	// Ctor
	ThreadPool::Task::Task(Priority priority): m_priority(priority)
	{}
	
	// bool operator<
	bool ThreadPool::Task::operator<(const Task& other)
	{
		return (this->m_priority < other.m_priority);
	}

// ======================== operator < task pointers =========================//
// operator<
	bool ThreadPool::cmp_tasks( const std::shared_ptr<ThreadPool::Task> left,
                                const std::shared_ptr<ThreadPool::Task> right)
	{
		// calling Task::operator<
		return (*left < *right);
	}

//========================== KillMeTask ======================================//
// class def
	class ThreadPool::KillMeTask : public ThreadPool::Task
	{
	public:
		KillMeTask(ThreadPool* ptr);
		~KillMeTask() = default;
		
	private:
		ThreadPool* m_tp_ptr;
		void execute() noexcept;
	};
	
// Ctor
	ThreadPool::KillMeTask::KillMeTask(ThreadPool* ptr) :
		Task(Task::Priority::SYSTEM),
		m_tp_ptr(ptr)
	{}
	
// execute (bound to kill_thread func)
	void ThreadPool::KillMeTask::execute() noexcept
	{
		// get current id
		std::thread::id this_id = std::this_thread::get_id();
		
		// set flag to false
		m_tp_ptr->m_flags_map.at(this_id) = false;
		
		// let the kill_thread function continue
		m_tp_ptr->m_sem.post();
	}

	
//======================== ThreadPool ========================================//
// Ctor
	ThreadPool::ThreadPool(size_t thread_num, std::chrono::milliseconds timeout):
		m_queue(cmp_tasks),
		m_threads_vec(thread_num),
		m_timeout(timeout),
		m_suspended(false)
	{
		ScopeLock<std::mutex> lock(m_mutex);

		for (auto& it : m_threads_vec)
		{
			it = std::thread(&ThreadPool::thread_func, this);
			m_flags_map.insert({it.get_id(), true});
		}
	}
	
// Dtor
	ThreadPool::~ThreadPool()
	{
		resize(0);
		std::cout << "ThreadPool::Dtor end" << std::endl;
	}
	
// add_task
	void ThreadPool::add_task(std::shared_ptr<ThreadPool::Task> task)
	{
		m_queue.push(task);
	}
	
// resize
	void ThreadPool::resize(size_t new_size)
	{
		int dif = new_size - m_threads_vec.size();
		
		if (dif < 0)
		{
			decrement(abs(dif));
		}
		else if (dif > 0)
		{
			increment(dif);
		}
		// else - do nothing
	}

// stop
	void ThreadPool::stop()
	{
		resize(0);
	}

// suspend
	void ThreadPool::suspend()
	{
		m_suspended = true;
		size_t threads_amount = m_threads_vec.size();
		
		for (size_t i = 0; i < threads_amount; ++i)
		{
			m_sem.wait();
		}
		// make sure all the threads are in the barrier
		ScopeLock<std::mutex> lock(m_mutex);
	}

// resume
	void ThreadPool::resume()
	{
		m_suspended = false;
		m_cond_var.notify_all();
	}

// increment
	void ThreadPool::increment(size_t dif)
	{
		// in a loop: push new threads into the vector + flags map
		for (size_t i = 0; i < dif; ++i)
		{
			std::thread temp_thread(&ThreadPool::thread_func, this);
			
			ScopeLock<std::mutex> lock(m_mutex);
			m_flags_map.insert({temp_thread.get_id(), true});
			m_threads_vec.push_back(std::move(temp_thread));
		}
	}


// decrement
	void ThreadPool::decrement(size_t dif)
	{
		// in a loop: kill_thread
		for (size_t i = 0; i < dif; ++i)
		{
			kill_thread();
		}
	}

// kill_thread (not MT-safe)
	void ThreadPool::kill_thread()
	{
		// is there at least 1 thread alive in the vector?
		if (!m_threads_vec.empty())
		{
			// creates task with 'this'
			std::shared_ptr<KillMeTask> kill_task(new KillMeTask(this));
			
			// push to PQ
			add_task(kill_task);
			
			// wait till a thread is ready to be kiilled
			m_sem.wait();
			
			// traverse the map, searching for the false flag. (there must be 
			// at least 1)
			auto iter = m_flags_map.begin();
			for (; iter->second != false; ++iter );		
			
			// find the thread to kill
			for (auto it = begin(m_threads_vec); it != end(m_threads_vec); ++it)
			{
				// checks the thread's ID matches
				if (it->get_id() == iter->first)
				{
					// erase thread from the vector
					it->join();
					m_threads_vec.erase(it);
					
					// erase thread from the map + end loop
					m_flags_map.erase(iter);
					break;
				}
			}
		}
	}

// static member initializer
	const std::chrono::milliseconds ThreadPool::default_timeout = {std::chrono::milliseconds(10)};

// === thread_func ===
	void ThreadPool::thread_func()
	{		
		try
		{
			// get current id
			std::thread::id this_id = std::this_thread::get_id();
			bool keep_run = true;
			
			while (keep_run)
			{
				// check if need to stop
				if (m_suspended)
				{
					ScopeLock<std::mutex> lock(m_mutex);
					m_sem.post();
					m_cond_var.wait(m_mutex);
				}
				
				// prepare an outparam for the WPQ
				std::shared_ptr<Task> task_ptr(nullptr);
				
				// pop task from the pq
				if (m_queue.pop<std::chrono::milliseconds>(task_ptr, m_timeout))
				{
					task_ptr->execute();
					
					// update the flag
					ScopeLock<std::mutex> lock(m_mutex);
					keep_run = m_flags_map.at(this_id);
				}
			}
		}
		catch(const std::exception& e)
		{
			std::cout << "ThreadPool::thread_func: catched!" << std::endl;
			std::cout << e.what() << std::endl;
		}
	}




}// end of hrd8