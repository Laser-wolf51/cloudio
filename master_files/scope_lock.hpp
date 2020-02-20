// -----------------------------------------------------------------------------
// File name  : scope_lock.hpp
// Developer  : Eyal Weizman
// Date       : 2019-06-16
// Reviewer   : 
// Description: scope_lock header.
// HOW TO USE : ScopeLock<std::mutex> lock(m_mutex);
// -----------------------------------------------------------------------------
#ifndef __ILRD_SCOPE_LOCK_HPP__
#define __ILRD_SCOPE_LOCK_HPP__

#include <iostream>
#include <pthread.h>	// mutex_init, mutex_destroy....
#include <mutex>        // std::mutex

namespace hrd8
{

//====================== template class mutex ================================//
template <typename T>
class ScopeLock
{
public:
	// Ctor
	explicit ScopeLock(T& mutex): m_mutex(mutex)
	{
		m_mutex.lock();
	}
	
	// Dtor
	~ScopeLock()
	{
		m_mutex.unlock();
	}
	
	ScopeLock(const ScopeLock& other) = delete;
	ScopeLock& operator=(const ScopeLock& other) = delete;

private:
	T& m_mutex;	
};


//====================== C speciated version =================================//
// C Ctor (inline to prevent strong symbol)
template<>
inline ScopeLock<pthread_mutex_t>::ScopeLock(pthread_mutex_t& mutex): m_mutex(mutex)
{
	pthread_mutex_lock(&m_mutex);
}

// D Dtor (inline to prevent strong symbol)
template<>
inline ScopeLock<pthread_mutex_t>::~ScopeLock()
{
	pthread_mutex_destroy(&m_mutex);
}


}// end of hrd8


#endif // __ILRD_SCOPE_LOCK_HPP__