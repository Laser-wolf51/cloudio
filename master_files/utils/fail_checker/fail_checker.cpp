// -----------------------------------------------------------------------------
// File name  : fail_checker.cpp
// Developer  : Eyal Weizman
// Description: fail_checker source file
// -----------------------------------------------------------------------------
#include <cerrno>

#include "fail_checker.hpp"

namespace hrd8
{

// check_fail
void check_fail(int ret_val, std::string err_message)
{
    if (ret_val < 0)
	{
		throw std::runtime_error(err_message);
	}
}

int convert_ptr(void* ptr)
{
	return ( (ptr == nullptr) ? (-1) : (0));
}

int convert_bool(bool val)
{
	return ( (val == false) ? (-1) : (0));
}


}// end of hrd8