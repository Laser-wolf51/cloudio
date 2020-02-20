// -----------------------------------------------------------------------------
// File name  : wpq_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-14
// Description: wpq test file
// -----------------------------------------------------------------------------
#include <iostream>
#include <thread>

#include "wpq.hpp"

using namespace hrd8;

//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n============= wpq test ==============\n" << std::endl;
	PriorityQueue<int> pq;
	int arr_input[4] = {12, 8, 4, 0};
	int arr_output[4] = {0};
	
	pq.push(arr_input[3]);
	pq.push(arr_input[1]);
	pq.push(arr_input[2]);
	pq.push(arr_input[0]);
	
	size_t full_size = pq.size();
	
	int i = 0;
	while (!pq.empty())
	{
		pq.pop<std::chrono::seconds>(arr_output[i], std::chrono::seconds(3));
		++i;
	}
	
	// multi-test
	std::cout << "multi-test:\t\t";
	(arr_input[0] == arr_output[0]) &&
	(arr_input[1] == arr_output[1]) &&
	(arr_input[2] == arr_output[2]) &&
	(arr_input[3] == arr_output[3]) &&
	(full_size == 4)
	?
	std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	
	// pop when empty
	std::cout << "pop when empty:\t\t";
	std::chrono::milliseconds timeout(1200);
	auto start = std::chrono::high_resolution_clock::now();
	bool status =  pq.pop<std::chrono::milliseconds>
		(arr_output[0], timeout);
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// now lets check:
	(duration.count() == timeout.count()) &&
	(status == false)
	?
	std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	// std::cout << "\n dur.count(): " << duration.count() << std::endl;
	// std::cout << "status: " << status << std::endl;
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}
