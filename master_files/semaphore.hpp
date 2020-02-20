// -----------------------------------------------------------------------------
// File name  : semaphore.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-14
// Description: semaphore header
// -----------------------------------------------------------------------------
#ifndef __ILRD_SEMAPHORE_HPP__
#define __ILRD_SEMAPHORE_HPP__

#include <semaphore.h>  // c library
#include <cerrno>
#include <stdexcept>    //runtime_error
#include <ctime>
#include <chrono>
#include <iostream>
namespace hrd8
{
//========================== Semaphore =======================================//
class Semaphore
{
public:
	enum status_t
	{
		success = 0,
		failure = -1
	};
	
	// Ctor
	explicit Semaphore(int initial_val = 0);
	~Semaphore();

	void post();
	void wait();
	status_t trywait();
	
	// TimeUnit must be from std::chrono
	// throws exception runtime_error
	template <typename TimeUnit>
	enum status_t timedwait(TimeUnit timeout);
	
	Semaphore(const Semaphore& other) = delete;
	Semaphore& operator=(const Semaphore& other) = delete;
	
private:
	sem_t m_sem;
};


template <typename TimeUnit>
enum Semaphore::status_t Semaphore::timedwait(TimeUnit timeout)
{
	// count how much nsec there is in timeout
	long wait_nsec = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
    struct timespec abs_timeout;
	const int billion = 1000000000;
	
    // get the current time into abs_timeout
    if (clock_gettime(CLOCK_REALTIME, &abs_timeout) == -1)
    {
        throw std::runtime_error("semaphore timedwait error: cannot create timer");
    }
    
    long total_nsec = abs_timeout.tv_nsec + wait_nsec;
	abs_timeout.tv_nsec = total_nsec % billion;
    abs_timeout.tv_sec += total_nsec / billion;
		
	status_t ret_val = static_cast<status_t>(sem_timedwait(&m_sem, &abs_timeout));
	
    return (ret_val);
}

}// end of hrd8


#endif // __ILRD_SEMAPHORE_HPP__