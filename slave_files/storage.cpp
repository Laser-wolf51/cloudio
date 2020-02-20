// -----------------------------------------------------------------------------
// File name  : storage.cpp
// Developer  : Eyal Weizman
// Date       : 2019-09-01
// Description: storage source file
// -----------------------------------------------------------------------------

// Note: the FS does not send another request to a memory block before it gets 
// back the replyfrom it.
// meaning: the storage does not have to be MT-safe.

#include <stdio.h>      // perror
#include <sys/types.h>  // open
#include <sys/stat.h>   // open
#include <fcntl.h>      // open
#include <unistd.h>     // close
#include <unistd.h>     // write

#include "check_fail.hpp"
#include "storage.hpp"

namespace hrd8
{
// Ctor
Storage::Storage(size_t slave_size, const std::string& store_path)
	:
	m_slave_size(slave_size),
	m_path(store_path),
	m_storage_fd(0)
{
	m_storage_fd = open(m_path.c_str(), O_RDWR);
    check_fail(m_storage_fd, "Storage Ctor: open failed"); 
}

// Dtor
Storage::~Storage()
{
    close(m_storage_fd);
}

// read
std::unique_ptr<DriverProxyBase::DriverData>
Storage::read(std::unique_ptr<DriverProxyBase::DriverData> read_request)
{
    // std::cout << "storage: start read. m_slave_size = " << m_slave_size << std::endl;
    check_fail(
        lseek(m_storage_fd, read_request->get_offset(), SEEK_SET), 
        "Storage read: lseek failed");
    
    int bytes_read = 0;
    int left_to_read = read_request->get_len();
    char* vector_buffer = read_request->get_buffer().data();


    std::cout << "" << std::endl;
    // reading
    do
    {
        bytes_read = ::read(m_storage_fd, vector_buffer, left_to_read);
        left_to_read -= bytes_read;
        vector_buffer += bytes_read;

    } while ((bytes_read > 0) && (left_to_read > 0));  

    // making sure everything was read
    if (bytes_read == -1)
    {
        perror("storage");
        throw std::runtime_error("Storage::read: read failed");
    }
    
    // debug
    // std::cout << "storage read: read amount = " << (read_request->get_len() - left_to_read) << std::endl;

    return (std::move(read_request));
}

// write
std::unique_ptr<DriverProxyBase::DriverData>
Storage::write(std::unique_ptr<DriverProxyBase::DriverData> write_request)
{
    // std::cout << "storage: start write" << std::endl;
    check_fail(
        lseek(m_storage_fd, write_request->get_offset(), SEEK_SET), 
        "Storage write: lseek failed");
    
    int bytes_written = 0;
    int left_to_write = write_request->get_len();
    char* vector_buffer = write_request->get_buffer().data();

    // writing
    do
    {
        bytes_written = ::write(m_storage_fd, vector_buffer, left_to_write);
        left_to_write -= bytes_written;
        vector_buffer += bytes_written;
    
    } while ((bytes_written > 0) && (left_to_write > 0));  

    // making sure everything was read
    if (bytes_written == -1)
    {
        perror("storage");
        throw std::runtime_error("Storage::read: read failed");
    }

    // std::cout << "storage write: write amount = " 
	// << (write_request->get_len() - left_to_write) << std::endl;
    return (std::move(write_request));
}

// get_slave_size
size_t Storage::get_slave_size()
{
	return (m_slave_size);
}

}// end of hrd8




// graveyard

// std::unique_ptr<DriverProxyBase::DriverData>
// Storage::read(std::unique_ptr<DriverProxyBase::DriverData> read_request)
// {
//     m_fstream.open(m_path);
//     m_fstream.seekg(read_request->get_offset(), std::ios::beg);
//     m_fstream.read(read_request->get_buffer().data(), read_request->get_len());

//     m_fstream.close();

//     return (std::move(read_request));
// }
