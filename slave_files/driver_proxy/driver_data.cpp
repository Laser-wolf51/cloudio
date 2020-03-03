// -----------------------------------------------------------------------------
// File name  : driver_data.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-20
// Description: DriverData source file. part of the abstract class driver_proxy
// -----------------------------------------------------------------------------
#include <string>       // std::string

#include "driver_proxy.hpp"

namespace hrd8
{
//========================== DriverData ======================================//
// Ctor
    DriverProxyBase::DriverData::DriverData(size_t offset, unsigned int len,
        int req_id, action_type type) :
    m_offset(offset),
    m_len(len),
    m_req_id(req_id),
    m_type(type)
    {}

// get offset
	size_t DriverProxyBase::DriverData::get_offset()
	{
		return (m_offset);
	}

// set_offset
	void DriverProxyBase::DriverData::set_offset(size_t new_offset)
	{
		m_offset = new_offset;
	}

// get len
	unsigned int DriverProxyBase::DriverData::get_len()
	{
		return (m_len);
	}
    
// get_id
    int DriverProxyBase::DriverData::get_id()
    {
        return (m_req_id);
    }

// get type
    DriverProxyBase::DriverData::action_type DriverProxyBase::DriverData::get_type()
    {
        return (m_type);
    }


// get buffer
    std::vector<char>& DriverProxyBase::DriverData::get_buffer()
    {
        return (m_buffer);
    }



}// end of hrd8

