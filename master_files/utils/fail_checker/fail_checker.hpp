// -----------------------------------------------------------------------------
// File name  : fail_checker.hpp
// Developer  : Eyal Weizman
// Description: fail_checker header
// -----------------------------------------------------------------------------
#ifndef __ILRD_FAIL_CHECKER_HPP__
#define __ILRD_FAIL_CHECKER_HPP__

#include <iostream> // std::string, runtime_error

namespace hrd8
{
// if ret_val is < 0 - throws a runtime_error with err_message
void check_fail(int ret_val, std::string err_message);

int convert_ptr(void* ptr);	// nullptr == fail
int convert_bool(bool val); // flase == fail
	
}// end of hrd8


#endif // __ILRD_FAIL_CHECKER_HPP__