// -----------------------------------------------------------------------------
// File name  : driver_proxy_slave.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-20
// Description: DriverProxySlave source file. defines the udp connection between
// 				the slave and the master.
// -----------------------------------------------------------------------------
#include <unistd.h>     // write, close
#include <netdb.h>      // addrinfo
#include <cstring>      // memset, memcpy
#include <string>       // std::string
#include <iostream>

#include "../utils/scope_lock.hpp"
#include "../utils/fail_checker/fail_checker.hpp"
#include "driver_proxy_slave.hpp"

namespace hrd8
{
//========================== DriverProxySlave ================================//
// Ctor
	DriverProxySlave::DriverProxySlave(const std::string& server_ip, 
	int port_num, size_t message_size) 
		:
		m_server_ip(server_ip),
		m_port(port_num),
		m_message_size(message_size),
		m_master_sock(m_server_ip, m_port)
	{
	std::string init_msg = "Hi Master I am your slave";
	m_master_sock.send_msg(init_msg.c_str(), init_msg.size());
	
	std::cout << "DriverProxySlave Ctor: done connectiong" << std::endl;
	}


// Dtor
    DriverProxySlave::~DriverProxySlave()
    {
    	// disconnect 
        disconnect();
        
        std::cout << "DriverProxySlave Dtor: finished closing" << std::endl;
    }

// disconnect
    void DriverProxySlave::disconnect()
    {}

// receive_from_driver
    std::unique_ptr<DriverProxyBase::DriverData> DriverProxySlave::receive_from_driver()
    {
        std::vector<char> buffer(m_message_size);
        
        m_master_sock.receive(buffer.data(), m_message_size);
		
		// prepare str
        std::string buf_str(buffer.data(), m_message_size);
        std::cout << "receive_from_driver: header = " << buf_str.c_str() << std::endl;

        size_t str_pos = 0;
        size_t temp_pos = 0;

        // parse offset
        unsigned long offset = std::stol(buf_str, &str_pos);
        ++str_pos; // promote to next token

        // parse length
        int length = std::stoi(buf_str.substr(str_pos), &temp_pos);
        str_pos += temp_pos + 1; // promote the absulute pos to next token
        
        // parse req_id
        int req_id = std::stoi(buf_str.substr(str_pos), &temp_pos);
        str_pos += temp_pos + 1; // promote the absulute pos to next token

        // parse req_type
        int req_type = std::stoi(buf_str.substr(str_pos), &temp_pos);
        str_pos += temp_pos + 2; 

        // allocating the request on the heap
        std::unique_ptr<DriverData> data_ptr(
        new DriverData( offset,
                        length,
                        req_id,
                        static_cast<DriverData::action_type>(req_type) )
                        );
        
        // if the request isrto write - reading the data into buffer
        if (DriverData::WRITE == static_cast<DriverData::action_type>(req_type))
        {
            // get buffer frrm data
            std::vector<char>& data_buf = data_ptr->get_buffer();

            // prepare buffer size
            data_buf.resize(length);

            // copy from right after the \0 to the buffer - length bytes
            buf_str.copy(data_buf.data(), length, str_pos);
        }

        return (std::move(data_ptr));
    }

// send_to_driver
    void DriverProxySlave::send_to_driver(std::unique_ptr<DriverData> data)
    {
        std::string buf_str;
        // append len
        buf_str += std::to_string(data->get_len());
        buf_str.push_back(';');

        // append req_id
        buf_str += std::to_string(data->get_id());
        buf_str.push_back(';');
        
        // append req_type
        buf_str += std::to_string(data->get_type());
        buf_str.push_back(';');
        buf_str.push_back('\0');

        size_t header_size = buf_str.size(); // for READ case
        buf_str.resize(m_message_size);

        // if the request is to READ - writng the data into the string
        if (DriverData::READ == data->get_type())
        {
            // get iter to right after the \0
            auto dest_iter = buf_str.begin() + header_size;

            // copy from vector to the string
            std::copy(data->get_buffer().begin(), data->get_buffer().end(), dest_iter);
        }

        // write_to_sock(buf_str.c_str(), m_message_size);
        m_master_sock.send_msg(buf_str.c_str(), m_message_size);
    }

// get request fd
    int DriverProxySlave::get_socket_fd()
    {
        return (m_master_sock.get_socket());
    }

}// end of hrd8


