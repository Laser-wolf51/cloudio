// -----------------------------------------------------------------------------
// File name  : scope_lock_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-06-22
// Reviewer   : 
// Description: scope_lock test file
// -----------------------------------------------------------------------------
#include <iostream>
#include <mutex>        // std::mutex

#include "scope_lock.hpp"

using namespace hrd8;

// fwd declaration
static void scope_test();

//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n-------------- scope_lock test -------------\n" << std::endl;
	std::cout << "============================================\n" << std::endl;
	
	scope_test();
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}


// ====================== scope_test ============================/
static void scope_test()
{
	pthread_mutex_t mut1;
	pthread_mutex_init(&mut1, NULL);
	
	std::mutex mut2;
	std::recursive_mutex mut3;
	
	ScopeLock<pthread_mutex_t> l1(mut1);
	ScopeLock<std::mutex> l2(mut2);
	ScopeLock<std::recursive_mutex> l3(mut3);
	
}

