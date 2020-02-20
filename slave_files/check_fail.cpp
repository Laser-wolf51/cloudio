// -----------------------------------------------------------------------------
// File name  : check_fail.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-
// Description: check_fail source file
// -----------------------------------------------------------------------------
#include <cerrno>
#include "check_fail.hpp"

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