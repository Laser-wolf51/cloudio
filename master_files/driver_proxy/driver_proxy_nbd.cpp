// -----------------------------------------------------------------------------
// File name  : driver_proxy_nbd.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-20
// Description: DriverProxyNBD source file
// -----------------------------------------------------------------------------
#include <fcntl.h>      // open
#include <unistd.h>     // write, close
#include <sys/ioctl.h>  // ioctl
#include <sys/socket.h> // socketpair
#include <arpa/inet.h>  // ntohl
#include <linux/nbd.h>  // NBD_SET_SIZE, NBD_CLEAR_SOCK
#include <signal.h>     // sigfillset, sigprocmask
#include <cstring>      // memset, memcpy
#include <string>       // std::string

#include "../utils/scope_lock.hpp"
#include "../utils/fail_checker/fail_checker.hpp"
#include "driver_proxy_nbd.hpp"

namespace hrd8
{
//========================== DriverProxyNBD ==================================//
// Ctor
    DriverProxyNBD::DriverProxyNBD(
        const std::string& dev_file, 
        size_t storage_size) :
        m_socket_fd(0),
        m_dissconnect(false),
        m_id_counter(0)
    {
    // creates 2 connected sockets
    	int sock_fd_arr[2];
    	check_fail(socketpair (AF_UNIX, SOCK_STREAM, 0, sock_fd_arr),
    	   "socketpair failed");
    	
    	// assign to member file descriptors
    	m_socket_fd = sock_fd_arr[0];
    	m_device_fd = sock_fd_arr[1];
    	
    	// open the device in dev_file
    	check_fail(m_nbd_fd = open(dev_file.c_str(), O_RDWR), "open NBD failed");
    
    	// set nbd size
    	check_fail(ioctl(m_nbd_fd, NBD_SET_SIZE, storage_size), 
    	   "NBD_SET_SIZE failed");
    	
    	// clear socket (?)
    	check_fail(ioctl(m_nbd_fd, NBD_CLEAR_SOCK), "NBD_CLEAR_SOCK failed"); 
    	
    	// creating a independent thread to do "NBD_DO_IT"
        try
        {
    	   m_thread = std::thread (&DriverProxyNBD::nbd_thread, this);
        }
        catch(const std::exception& e)
        {
            check_fail(-1, "creating thread failed");
        }
        std::cout << "DriverProxyNBD::Ctor: done connectiong" << std::endl;
    }

// nbd_thread
    void DriverProxyNBD::nbd_thread()
    {
    	// blocking all signals
        sigset_t sigset;
        int flags = 0;
    	
        check_fail(sigfillset(&sigset), "DriverProxyNBD: Sigfillset Failed");
        check_fail(sigprocmask(SIG_SETMASK, &sigset, NULL), 
            "DriverProxyNBD: Sigprocmask Failed\n");

        // setting nbd sockets and flags
        check_fail(ioctl(m_nbd_fd, NBD_SET_SOCK, m_device_fd), 
            "DriverProxyNBD: NBD_SET_SOCK failed\n"); 
                
        // optional:
        // flags |= NBD_FLAG_SEND_TRIM;
        // flags |= NBD_FLAG_SEND_FLUSH;
        check_fail(ioctl(m_nbd_fd, NBD_SET_FLAGS, flags), 
            "DriverProxyNBD: NBD_SET_FLAGS Faild"); 
                
        // nbd do it - that's the main part! creating the conncection
        check_fail(ioctl(m_nbd_fd, NBD_DO_IT), 
            "DriverProxyNBD: NBD_DO_IT Terminated with error"); 
                
        // cleaning up after disconnect
        check_fail(ioctl(m_nbd_fd, NBD_CLEAR_QUE), 
            "DriverProxyNBD: NBD_CLEAR_QUEUE Failed");
        check_fail(ioctl(m_nbd_fd, NBD_CLEAR_SOCK), 
            "DriverProxyNBD: NBD_CLEAR_SOCK Failed"); 
        
        std::cout << "NBD thread: disconnected" << std::endl;
    }

// Dtor
    DriverProxyNBD::~DriverProxyNBD()
    {
    	// disconnect 
        try
        {
            disconnect();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
        std::cout << "DriverProxyNBD Dtor: finished closing" << std::endl;
    }

// disconnect
    void DriverProxyNBD::disconnect()
    {
    	// disconnect
        if (m_dissconnect == false)
        {
            m_dissconnect = true;
            check_fail(ioctl(m_nbd_fd, NBD_DISCONNECT), "NBD Disconnect Failed\n");
            
            //close sockets
            close(m_socket_fd);
            close(m_device_fd);
            
            // wait for the thread
            m_thread.join();
            
            // close the device (Dtor should never throw exceptions)
            close(m_nbd_fd);
        }
    }

// receive_from_driver
    std::unique_ptr<DriverProxyBase::DriverData> DriverProxyNBD::receive_from_driver()
    {
        struct nbd_request request;
        
        // read a request from the socket into 'request'
        read_from_sock(reinterpret_cast<char* >(&request), sizeof(request));
        
        size_t offset = ntohll(request.from);
        unsigned int len = ntohl(request.len);
        
        // get the request type
        DriverData::action_type req_type = DriverData::check_type(ntohl(request.type));
        
        // allocating the request on the heap
        // may throw bad_alloc
        std::unique_ptr<DriverData>request_ptr(new DriverData(
            offset, 
            len, 
            convert_handle_to_id(std::string(request.handle, 8)),
            req_type));
        
        // resizing the buffer according to the len in the request
        std::vector<char>& buffer = request_ptr->get_buffer();
        buffer.resize(len);
        
        // if the request is to write - reading the data into buffer
        if  (DriverData::WRITE == req_type)
        {
            read_from_sock(&buffer[0], len);
        }
		
		// returning the pointer with std::move
        return (std::move(request_ptr));
    }

// send_to_driver
    void DriverProxyNBD::send_to_driver(std::unique_ptr<DriverData> data)
    {
        struct nbd_reply reply;
        memset(&reply, 0, sizeof(reply));

        // updating reply parameters
        reply.magic = htonl(m_reply_magic_number);
        
        memcpy(reply.handle, convert_id_to_handle(data->get_id()).c_str(), 8);  
        
        // very important!!!! only 1 thread at a time may write to NBD
        ScopeLock<std::mutex> lock(m_mutex);

        // sending the reply
        write_to_sock(reinterpret_cast<char* >(&reply), sizeof(reply));
        
        // if the original request was to read, writing back the data
        if (DriverData::action_type::READ == data->get_type())
        {
            std::vector<char>& buffer = data->get_buffer();
            unsigned int len = data->get_len();
            
            write_to_sock(&buffer[0], len);
        }  
    }


// get_socket_fd
    int DriverProxyNBD::get_socket_fd()
    {
        return (m_socket_fd);
    }





// read all
    void DriverProxyNBD::read_from_sock(char* buffer, unsigned int count)
    {
        size_t bytes_read = 0;
        
        // reading    
        while ((bytes_read = read(m_socket_fd, buffer, count)) > 0)
        {
            buffer += bytes_read;
            count -= bytes_read;
        }
        
        // making sure everything was read
        if (count != 0)
        {
            throw std::runtime_error("DriverProxyNBD: read failed");
        }
    }


// write all
    void DriverProxyNBD::write_to_sock(const char* buffer, unsigned int count)
    {
        size_t bytes_written = 0;

        // writing
        while ((bytes_written = write(m_socket_fd, buffer, count)) > 0)
        {
            buffer += bytes_written;
            count -= bytes_written;
        }
        
        // making sure all was written
        if (count != 0)
        {
            throw std::runtime_error("DriverProxyNBD: write failed");
        }
    }

// convert_handle_to_id
    int DriverProxyNBD::convert_handle_to_id(const std::string& handle)
    {
        ScopeLock<std::mutex> lock(m_id_converter_mutex);
        ++m_id_counter;
        m_handles_map.insert({m_id_counter, handle});
        return (m_id_counter);
    }

// convert_id_to_handle
    std::string DriverProxyNBD::convert_id_to_handle(int req_id)
    {
        ScopeLock<std::mutex> lock(m_id_converter_mutex);
        std::string ret_val = m_handles_map[req_id];
        m_handles_map.erase(req_id);
        return (ret_val);
    }

// check type
    DriverProxyBase::DriverData::action_type
                     DriverProxyBase::DriverData::check_type(unsigned int type)
    {
        switch (type)
        {
            case NBD_CMD_READ:
                return READ;
    
            case NBD_CMD_WRITE:
                return WRITE;
    
            case NBD_CMD_DISC:
                return DISC;
                
            case NBD_CMD_FLUSH:
                return FLUSH;
    
            case NBD_CMD_TRIM:
                return TRIM;
    
            default: throw std::runtime_error("unknown request type recieved");
        }
    }

// ntohll
    size_t DriverProxyNBD::ntohll(size_t a)
    {
        size_t lo = a & 0xffffffff;
        size_t hi = a >> 32U;
        lo = ntohl(lo);
        hi = ntohl(hi);
    
        return ((lo) << 32U | hi);
    }

}// end of hrd8