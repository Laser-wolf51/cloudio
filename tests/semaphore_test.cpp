// -----------------------------------------------------------------------------
// File name  : semaphore_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-06-23
// Description: semaphore test file
// -----------------------------------------------------------------------------
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include "semaphore.hpp"

using namespace hrd8;
//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n-------------- semaphore test -------------\n" << std::endl;
	std::cout << "============================================\n" << std::endl;
	
	Semaphore s1;
	s1.post();
	s1.wait();
	
	// trywait
	std::cout << "trywait:\t";
	(s1.trywait() == Semaphore::failure)
	?
	std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	
	// timedwait
	std::cout << "timedwait:\t";
	int timeout = 1200;
	auto start = std::chrono::high_resolution_clock::now();
	Semaphore::status_t status =  s1.timedwait<std::chrono::milliseconds>
		(std::chrono::milliseconds(timeout));
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// now lets check:
	// std::cout << "\n dur.count(): " << duration.count() << std::endl;
	// std::cout << "status: " << status << std::endl;
	(duration.count() == timeout) &&
	(status == Semaphore::failure)
	?
	std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}
