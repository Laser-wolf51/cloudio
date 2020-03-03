// -----------------------------------------------------------------------------
// File name  : gateways.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-25
// Description: gateways source file
// -----------------------------------------------------------------------------

#include "../request_engine.hpp"
#include "../../slaves_manager/slaves_manager.hpp"
#include "gateways.hpp"

namespace hrd8
{

//===================== class FromNBDGate =================================//
// Ctor
FromNBDGate::FromNBDGate(std::shared_ptr<DriverProxyNBD> nbd_ptr,
							std::shared_ptr<SlavesManager> s_m_ptr)
	:
	m_nbd_ptr(nbd_ptr),
	sm_ptr(s_m_ptr)
{}

// extract_from_socket
std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>
	FromNBDGate::extract_from_socket()
{
	// std::cout << "FromNBDGate: extract_from_socket" << std::endl;
	
	// get request
	std::unique_ptr<DriverProxyNBD::DriverData> req_ptr = 
		m_nbd_ptr->receive_from_driver();

	if (req_ptr->get_type() == DriverProxyBase::DriverData::action_type::DISC)
	{
		m_nbd_ptr->disconnect();
	}
	
	// create Args object
	std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>
		args_ptr(new RequestEngine::RETaskBase::ArgsBase
			(std::move(req_ptr), nullptr, sm_ptr));
	
	// key val for the factory
	size_t key_val = SlavesManager::tasks_keys::TO_CLOUD_TASK;
	
	// std::cout << "FromNBDGate: extract_from_socket done" << std::endl;
	
	return (std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::
		ArgsBase>>(key_val, std::move(args_ptr)) );
}


//===================== class FromCloudGate =================================//
// Ctor
FromCloudGate::FromCloudGate(std::shared_ptr<DriverProxyNBD> nbd_ptr,
							std::shared_ptr<SlavesManager> s_m_ptr,
							int socket_fd,
							size_t slave_index)
	:
	m_nbd_ptr(nbd_ptr),
	sm_ptr(s_m_ptr),
	m_socket_fd(socket_fd),
	m_slave_index(slave_index)
{}

// extract_from_socket
std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>
	FromCloudGate::extract_from_socket()
{
	// std::cout << "FromCloudGate: extract_from_socket" << std::endl;
	
	// get request
	std::unique_ptr<DriverProxyNBD::DriverData> req_ptr = 
		sm_ptr->receive_from_driver(m_slave_index);

	// create Args object
	std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>
		args_ptr(new RequestEngine::RETaskBase::ArgsBase
			(std::move(req_ptr), m_nbd_ptr.get(), nullptr));
	
	// key val for the factory
	size_t key_val = SlavesManager::tasks_keys::TO_NBD_TASK;
	
	// std::cout << "FromCloudGate: extract_from_socket done" << std::endl;
	
	return (std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::
		ArgsBase>>(key_val, std::move(args_ptr)) );
}

// get_fd
int FromCloudGate::get_fd()
{
	return (m_socket_fd);
}

}// end of hrd8