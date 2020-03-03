// -----------------------------------------------------------------------------
// File name  : slaves_manager.hpp
// Developer  : Eyal Weizman
// Date       : 2019-08-26
// Description: slaves_manager header
// -----------------------------------------------------------------------------
#ifndef __ILRD_SLAVES_MANAGER_HPP__
#define __ILRD_SLAVES_MANAGER_HPP__

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>

#include "../driver_proxy/driver_proxy.hpp"
#include "udp_util/udp_util.hpp"


namespace hrd8
{

//===================== Class SlavesManager ===================================//
class SlavesManager
{

public:
	SlavesManager(const std::string& server_ip, 
					size_t slave_size,
					size_t num_of_slaves, 
					size_t msg_size,
					std::chrono::seconds wait_for_slave_timeout,
					std::chrono::seconds timeout_before_resend);
	
	~SlavesManager();
	
	// returns: the new socket fd + slave_index
	std::pair<int, size_t> add_slave(int port_num);
	
	// send a reply to a matcing request 
	void send_to_driver(std::unique_ptr<DriverProxyBase::DriverData> data);
	
	// blocks until there is an available request in the socket. when there is -
	// reads the request and returns it via ptr
	std::unique_ptr<DriverProxyBase::DriverData> receive_from_driver(size_t slave_index); 
	
	void disconnect();
	int get_socket_fd();
    
	enum tasks_keys
	{
		TO_CLOUD_TASK,
		TO_NBD_TASK
	};

private:
	class DataTracker;
	
	// Udp data members
	const std::string m_server_ip;
    const size_t m_slave_size;
	const size_t m_num_of_slaves;
    const size_t m_message_size;
	const std::chrono::seconds m_wait_for_slave_timeout;
	
	// tracking & resend data members
	const std::chrono::seconds m_timeout_before_resend;
	std::unordered_map<int,	std::shared_ptr<DataTracker>> m_tracking_map;
    std::mutex m_tracking_map_mutex;
	int m_thread_keep_run;
	std::vector<std::unique_ptr<UdpServer>> m_servers_vec;
	std::thread m_resending_thread;
	
	bool msg_is_ok(const std::vector<char>& buffer);
	std::string serialize_data(std::unique_ptr<DriverProxyBase::DriverData> data_ptr);
	std::unique_ptr<DriverProxyBase::DriverData> deserialize_data(const std::string& serialized_data);
    void resending_thread();
    void keep_track_on_data(std::string& serialized_data, int req_id, size_t slave_index);
	void write_into_socket(const char* buffer, size_t size, size_t slave_index);
	size_t calc_slave_index(size_t offset);
    void send_backup(std::unique_ptr<DriverProxyBase::DriverData> backup_data);
	size_t get_next_slave_index(size_t slave_index);
    
	SlavesManager(const SlavesManager& other) = delete;
    SlavesManager& operator=(const SlavesManager& other) = delete;
    SlavesManager(SlavesManager&& other) = delete;
    SlavesManager& operator=(SlavesManager&& other) = delete;
};


//===================== Class DataTracker ====================================//
class SlavesManager::DataTracker
{
public:
	DataTracker(size_t slave_index, 
				std::chrono::time_point<std::chrono::system_clock> m_timeout,
				std::string serialized_data);
	~DataTracker() = default;
	
	const std::string& get_serialized_data();
	const std::chrono::time_point<std::chrono::system_clock>& get_timeout();
	void set_timeout(std::chrono::time_point<std::chrono::system_clock> timeout);
	size_t get_slave_index();

private:
	size_t m_slave_index;
	std::chrono::time_point<std::chrono::system_clock> m_timeout;
	std::string m_serialized_data;
	
	DataTracker(const DataTracker& other) = delete;
	DataTracker& operator=(const DataTracker& other) = delete;
	DataTracker(DataTracker&& other) = delete;
	DataTracker& operator=(DataTracker&& other) = delete;
};


	
}// end of hrd8
#endif // __ILRD_SLAVES_MANAGER_HPP__
