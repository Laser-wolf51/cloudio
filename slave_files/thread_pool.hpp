// -----------------------------------------------------------------------------
// File name  : thread_pool.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-20
// Description: thread_pool header file
// -----------------------------------------------------------------------------
#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <chrono>           // milliseconds
#include <thread>           // thread
#include <unordered_map>    // unordered_map
#include <condition_variable>// condition_variable
#include <functional>       // std::function

#include "wpq.hpp"

//=== Namespace
namespace hrd8
{

// =============================================================================
//                               class ThreadPool
// =============================================================================
class ThreadPool
{
public:
    class Task;
    
    // Ctor: create pool of threads + activate them
    // timeout is the max waiting time for a task when the PQ is empty. if
    // expired - prints to std:cerr.
    ThreadPool(size_t thread_num = std::thread::hardware_concurrency(), 
               std::chrono::milliseconds timeout = default_timeout);
	
	// rellease the threads
    ~ThreadPool();
    
    // add tasks to the thread-pool
    void add_task(std::shared_ptr<Task> task);
    
    // change the number of threads in the pool
    void resize(size_t new_size);
    
	// kill all the threads
    void stop();
    
    // prevents all threads from starting new tasks
    void suspend();
    
    // make all threads to be able to work again
    void resume();
    
    // uncopyable and unmovable
    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;
    ThreadPool& operator=(ThreadPool&& other) = delete;

private:
	// allows PQ to compare 2 shared ptrs
	static bool cmp_tasks(  const std::shared_ptr<ThreadPool::Task> left,
							const std::shared_ptr<ThreadPool::Task> right);
	class KillMeTask;

	PriorityQueue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>,
		std::function<bool(const std::shared_ptr<ThreadPool::Task> ,const std::shared_ptr<ThreadPool::Task>)>> m_queue;
	std::vector<std::thread> m_threads_vec;
	std::unordered_map<std::thread::id, bool> m_flags_map;
	std::chrono::milliseconds m_timeout;
	bool m_suspended;
	std::mutex m_mutex;
	Semaphore m_sem;
	std::condition_variable_any m_cond_var;

	static const std::chrono::milliseconds default_timeout;

	void thread_func();
	void kill_thread();
	void decrement(size_t dif);
	void increment(size_t dif);
};

// =============================================================================
//                           class ThreadPool::Task
// =============================================================================
class ThreadPool::Task // Abstract class
{
public:
    enum class Priority
    {
        LOW,
        MEDIUM,
        HIGH,
        SYSTEM // dont use it 
    };
    
    explicit Task(Priority priority = Priority::MEDIUM);

    bool operator<(const Task& other);
	
    virtual ~Task() = default;
    Task(const Task& other) = default;
    Task(Task&& other) = default;
    Task& operator=(const Task& other) = default;
    Task& operator=(Task&& other) = default;

private:
    Priority m_priority;
	
	// execute is private so that only 'friend class ThreadPool' could invoke it.
    virtual void execute() noexcept = 0;
    friend class ThreadPool;
};



} // hrd8

#endif //__THREAD_POOL_HPP__