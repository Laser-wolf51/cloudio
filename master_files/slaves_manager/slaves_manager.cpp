// -----------------------------------------------------------------------------
// File name  : slaves_manager.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-22
// Description: SlavesManager source file. defines the connection with all of
// 				the slaves.
// -----------------------------------------------------------------------------
#include <iostream>

#include "../utils/fail_checker/fail_checker.hpp"
#include "../utils/scope_lock.hpp"
#include "slaves_manager.hpp"

namespace hrd8
{
//========================== SlavesManager ===================================//
// Ctor
    SlavesManager::SlavesManager(const std::string& server_ip, 
                                    size_t slave_size,
									size_t num_of_slaves,
                                    size_t msg_size,
									std::chrono::seconds wait_for_slave_timeout,
									std::chrono::seconds timeout_before_resend) :
        m_server_ip(server_ip),
        m_slave_size(slave_size),
		m_num_of_slaves(num_of_slaves),
        m_message_size(msg_size),
		m_wait_for_slave_timeout(wait_for_slave_timeout),
		m_timeout_before_resend(timeout_before_resend),
		m_thread_keep_run(true)
    {
		// starting the resending_thread
		m_resending_thread = std::thread(&SlavesManager::resending_thread, this);
		
		std::cout << "SlavesManager Ctor: Done" << std::endl;
    }

// Dtor
	SlavesManager::~SlavesManager()
	{
		// stop & join the thread
		m_thread_keep_run = false;
		m_resending_thread.join();
		std::cout << "~SlavesManager: Done" << std::endl;
	}

// add_slave
	std::pair<int, size_t> SlavesManager::add_slave(int port_num)
	{
		std::unique_ptr<UdpServer> slave_server(new UdpServer(m_server_ip, port_num));
		std::vector<char> buffer(m_message_size, 0);
		
		std::cout << "add_slave: waiting for slave of port: " << port_num << std::endl;
		
		// wait for a starting massege from a slave
		int ret_val = slave_server->timed_receive(buffer.data(), m_message_size,
			m_wait_for_slave_timeout);
		check_fail(ret_val, "SlavesManager add_slave - timeout");
		
		bool ret_val_bool = msg_is_ok(buffer);
		check_fail(convert_bool(ret_val_bool), "SlavesManager Ctor: msg is *not* OK");
		
		// get the socket_fd
		int socket_fd = slave_server->get_socket();
		
		// get the slave_index. the current slave index is the vector size before adding.
		size_t slave_index = m_servers_vec.size();
		
		// adds the server to the vector
		m_servers_vec.push_back(std::move(slave_server));
		
		std::cout << "add_slave: slave of port " << port_num << " was added" << std::endl;
		
		return (std::pair<int, size_t>(socket_fd, slave_index));
	}

// msg_is_ok
    bool SlavesManager::msg_is_ok(const std::vector<char>& buffer)
    {
        std::string expected_msg("Hi Master I am your slave");
        std::string actual_msg(buffer.data(), expected_msg.size());
        // std::cout << "msg_is_ok: buffer = " << buffer.data() << std::endl;
        return (expected_msg == actual_msg);
    }

// send_to_driver
    void SlavesManager::
		send_to_driver(std::unique_ptr<DriverProxyBase::DriverData> orig_data)
    {
		std::cout << "send_to_driver: absulute offset = " << orig_data->get_offset() << std::endl;
		
		// only if its a write req - send for backup
		if (orig_data->get_type() == DriverProxyBase::DriverData::action_type::WRITE)
		{
			// create a backup copy of data
			std::unique_ptr<DriverProxyBase::DriverData> backup_data(new
				DriverProxyBase::DriverData(
					orig_data->get_offset(), 
					orig_data->get_len(), 
					orig_data->get_id(),
					orig_data->get_type()));
			
			send_backup(std::move(backup_data));
		}
		
		// get for later
		int req_id = orig_data->get_id();
		size_t raw_offset = orig_data->get_offset();
		
		// calc slave index
		size_t orig_slave_index = calc_slave_index(raw_offset);
		
		// reset the offset. origin must be in the range 0 - slave_size.
		orig_data->set_offset(raw_offset % m_slave_size);		
		
		// serialize datas
		std::string orig_buf_str = serialize_data(std::move(orig_data));
		
		// keep serialized data in map - only original!!!
		keep_track_on_data(orig_buf_str, req_id, orig_slave_index);
		
		std::cout << "send_to_driver: orig_buf_str = " << orig_buf_str.c_str() << std::endl;
		
		// write to socket
		write_into_socket(orig_buf_str.c_str(), m_message_size, orig_slave_index);
	}

// send_backup
	void SlavesManager::send_backup(std::unique_ptr<DriverProxyBase::DriverData> backup_data)
	{
		// for later
		size_t raw_offset = backup_data->get_offset();
		
		// calc the index of the backup slave.
		// backup must be in the range slave_size - slave_size * 2
		size_t backup_slave_index = get_next_slave_index(calc_slave_index(raw_offset));
		
		// reset the offset to the backup range
		backup_data->set_offset(raw_offset % m_slave_size + m_slave_size);
		
		// serialize datas
		std::string backup_buf_str = serialize_data(std::move(backup_data));
		
		std::cout << "send_to_driver: backup_buf_str = " << backup_buf_str.c_str() << std::endl;
		
		// write to socket
		write_into_socket(backup_buf_str.c_str(), m_message_size, backup_slave_index);
	}

// write_into_socket
	void SlavesManager::write_into_socket(const char* buffer, size_t size, size_t slave_index)
	{
		// send the string to the socket
		int ret_val = m_servers_vec.at(slave_index)->send_msg(buffer, size);
		check_fail(ret_val, "SlavesManager send: send_msg failed");
	}

// serialize_data
	std::string SlavesManager::serialize_data(std::unique_ptr<DriverProxyBase::DriverData> data_ptr)
	{
		// create string
        std::string buf_str;
        
        // append offset
		buf_str += std::to_string(data_ptr->get_offset());
		buf_str.push_back(';');
		
		// append len
        buf_str += std::to_string(data_ptr->get_len());
        buf_str.push_back(';');
		
		// append req_id
        buf_str += std::to_string(data_ptr->get_id());
        buf_str.push_back(';');
		
		// append req_type
        buf_str += std::to_string(data_ptr->get_type());
        buf_str.push_back(';');
        buf_str.push_back('\0');
		
		size_t header_size = buf_str.size(); // for WRITE case
		buf_str.resize(m_message_size);
		
		// if the request is to READ - writng the data into the string
		if (DriverProxyBase::DriverData::WRITE == data_ptr->get_type())
		{
			// get iter to right after the \0
			auto dest_iter = buf_str.begin() + header_size;
			
			// copy from vector to the string
			std::copy(data_ptr->get_buffer().begin(), 
				data_ptr->get_buffer().end(), dest_iter);
		}
		
		return (buf_str);
	}

// keep_track_on_data
	void SlavesManager::keep_track_on_data(std::string& serialized_data,
		int req_id, size_t slave_index)
	{
		// create a tracking object for the data
		std::shared_ptr<DataTracker> data_tracker_ptr(new DataTracker(
			slave_index,
			std::chrono::system_clock::now() + m_timeout_before_resend,
			serialized_data));
		
		// secure
		ScopeLock<std::mutex> lock(m_tracking_map_mutex);
		
		// add to map
		m_tracking_map.insert({req_id, data_tracker_ptr});
	}

// receive_from_driver
    std::unique_ptr<DriverProxyBase::DriverData> 
		SlavesManager::receive_from_driver(size_t slave_index)
    {
		// prepare buffer
		std::vector<char> buffer(m_message_size);
		
		// get msg from the slave socket to buffer
		int ret_val = m_servers_vec.at(slave_index)->receive(buffer.data(), m_message_size);
		check_fail(ret_val, "SlavesManager recv: receive failed");
		
		// convert to string
		std::string buf_str(buffer.data(), m_message_size);
		
		// debug
		std::cout << "receive_from_driver: header = " << buf_str.c_str() << std::endl;
		
		// deserialize
		std::unique_ptr<DriverProxyBase::DriverData> data_ptr = deserialize_data(buf_str);
		
		// update offset to original
		data_ptr->set_offset(data_ptr->get_offset() * (slave_index + 1));
		
		// debug
		std::cout << "receive_from_driver: original offset = " << 
			data_ptr->get_offset() << std::endl;

		// erase from map
		ScopeLock<std::mutex> lock(m_tracking_map_mutex);
		m_tracking_map.erase(data_ptr->get_id());
		
		return (std::move(data_ptr));
    }

// deserialize_data
	std::unique_ptr<DriverProxyBase::DriverData> 
		SlavesManager::deserialize_data(const std::string& serialized_data)
	{
		size_t str_pos = 0;
		size_t temp_pos = 0;
		
		// parse length
		int length = std::stoi(serialized_data.substr(str_pos), &temp_pos);
		str_pos += temp_pos + 1; // promote the absulute pos to next token

		// parse req_id
		int req_id = std::stoi(serialized_data.substr(str_pos), &temp_pos);
		str_pos += temp_pos + 1; // promote the absulute pos to next token

		// parse req_type
		int req_type = std::stoi(serialized_data.substr(str_pos), &temp_pos);
		str_pos += temp_pos + 2; 
		
		size_t offset = 0; // offset is irellevent in this direction
		
		// allocating the request on the heap
		std::unique_ptr<DriverProxyBase::DriverData> data_ptr(
		new DriverProxyBase::DriverData( offset,
						length,
						req_id,
						static_cast<DriverProxyBase::DriverData::action_type>(req_type) )
						);
		
		// if the request was to read - reading the data into buffer
		if (DriverProxyBase::DriverData::READ == static_cast<DriverProxyBase::DriverData::action_type>(req_type))
		{
			// get buffer frrm data
			std::vector<char>& data_buf = data_ptr->get_buffer();
			
			// prepare buffer size
			data_buf.resize(length);
			
			// copy from right after the \0 to the buffer - length bytes
			serialized_data.copy(data_buf.data(), length, str_pos);
		}
		
		return (std::move(data_ptr));
	}

// resending_thread
 void SlavesManager::resending_thread()
 {
	// std::cout << "SlavesManager::resending_thread: start" << std::endl;
	try
	{
		std::chrono::time_point<std::chrono::system_clock> next_wake_up =
			std::chrono::system_clock::now() + std::chrono::seconds(1);
		
		while (m_thread_keep_run)
		{
			std::this_thread::sleep_until(next_wake_up);
			
			ScopeLock<std::mutex> lock(m_tracking_map_mutex);
			
			for(auto& iter : m_tracking_map)
			{
				if (iter.second->get_timeout() < std::chrono::system_clock::now())
				{
					// resend the data
					write_into_socket(iter.second->
						get_serialized_data().c_str(), m_message_size,
						iter.second->get_slave_index());
					
					// reset the timeout
					iter.second->set_timeout(std::chrono::system_clock::now() +
						std::chrono::seconds(m_timeout_before_resend));
				}

				if (next_wake_up > iter.second->get_timeout())
				{
					next_wake_up = iter.second->get_timeout();
				}
			}
			
			// deinfe a default next_wake_up time point
			if (m_tracking_map.empty())
			{
				// wake up 1 second before any task might need a resend
				next_wake_up = std::chrono::system_clock::now() + 
				std::chrono::seconds(m_timeout_before_resend) - 
				std::chrono::seconds(1);
			}
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	std::cout << "SlavesManager::resending_thread: Done" << std::endl;
 }


// calc_slave_index
	size_t SlavesManager::calc_slave_index(size_t offset)
	{
		return (offset / m_slave_size);
	}

// get_next_slave_index
	size_t SlavesManager::get_next_slave_index(size_t slave_index)
	{
		++slave_index;
		if (slave_index >= m_num_of_slaves)
		{
			slave_index = 0;
		}
		
		return (slave_index);
	}

// disconnect
    void SlavesManager::disconnect()
    {}

//====================== SlavesManager::DataTracker ==========================//
// Ctor
	SlavesManager::DataTracker::DataTracker(size_t slave_index, 
		std::chrono::time_point<std::chrono::system_clock> m_timeout,
		std::string serialized_data)
		:
		m_slave_index(slave_index),
		m_timeout(m_timeout),
		m_serialized_data(serialized_data)
	{}

// get_serialized_data
	const std::string& SlavesManager::DataTracker::get_serialized_data()
	{return (m_serialized_data);}

// get_timeout
	const std::chrono::time_point<std::chrono::system_clock>& 
		SlavesManager::DataTracker::get_timeout()
	{return (m_timeout);}


// set_timeout
	void SlavesManager::DataTracker::
		set_timeout(std::chrono::time_point<std::chrono::system_clock> timeout)
	{m_timeout = timeout;}

// get_slave_index
	size_t SlavesManager::DataTracker::get_slave_index()
	{
		return (m_slave_index);
	}

}// end of hrd8


