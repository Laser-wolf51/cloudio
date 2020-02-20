// -----------------------------------------------------------------------------
// File name  : semaphore.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-
// Reviewer   : 
// Description: semaphore source file
// -----------------------------------------------------------------------------
#include <iostream>
#include "semaphore.hpp"

namespace hrd8
{
//========================== Semaphore ===============================//
// Ctor
	Semaphore::Semaphore(int initial_val)
	{
		sem_init(&m_sem, 0, initial_val);
	}
	
// Dtor
	Semaphore::~Semaphore()
	{
		sem_destroy(&m_sem);
	}
	
// post
	void Semaphore::post()
	{
		sem_post(&m_sem);
	}
	
// wait
	void Semaphore::wait()
	{
		sem_wait(&m_sem);
	}
	
// sem_trywait
	Semaphore::status_t Semaphore::trywait()
	{
		return (static_cast<status_t>(sem_trywait(&m_sem)));
	}
	

}// end of hrd8