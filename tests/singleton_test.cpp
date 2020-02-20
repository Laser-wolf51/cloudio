// -----------------------------------------------------------------------------
// File name  : singleton_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-11
// Description: singleton test file
// -----------------------------------------------------------------------------
#include <iostream>

#include "singleton.hpp"

using namespace hrd8;

//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n============= singleton test ==============\n" << std::endl;
	
	int* a = Singleton<int>::get_instance();
	int* b = Singleton<int>::get_instance();
	
	std::cout << "points to the same address:\t";
	(a == b)
	?
	std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}
