// -----------------------------------------------------------------------------
// File name  : storage.hpp
// Developer  : Eyal Weizman
// Date       : 2019-09-01
// Reviewer   : 
// Description: storage header
// -----------------------------------------------------------------------------
#ifndef __ILRD_STORAGE_HPP__
#define __ILRD_STORAGE_HPP__

#include <iostream>
#include <fstream>

#include "../driver_proxy/driver_proxy.hpp"

namespace hrd8
{
//========================== Storage ===============================//
class Storage
{
public:
	explicit Storage(size_t slave_size, const std::string& store_path);
	~Storage();
	
	std::unique_ptr<DriverProxyBase::DriverData>
		read(std::unique_ptr<DriverProxyBase::DriverData>);
	
	std::unique_ptr<DriverProxyBase::DriverData>        
		write(std::unique_ptr<DriverProxyBase::DriverData>);
	
	size_t get_slave_size();
	
private:
	const size_t m_slave_size;
	const std::string& m_path;
	int m_storage_fd;
};
	
}// end of hrd8

// graveyard
// void read(char *buffer, size_t offset, unsigned int len);
// void write(char *buffer, size_t offset, unsigned int len);

#endif // __ILRD_STORAGE_HPP__