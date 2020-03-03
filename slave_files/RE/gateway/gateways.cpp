// -----------------------------------------------------------------------------
// File name  : gateways.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-25
// Description: gateways source file
// -----------------------------------------------------------------------------

#include "gateways.hpp"

namespace hrd8
{

//====================== class SlaveGateway implementation ===================//
// Ctor
SlaveGateway::SlaveGateway(std::shared_ptr<DriverProxyBase> driver_prox_ptr,
							std::shared_ptr<Storage> storage_ptr)
	:
	m_driver_prox_ptr(driver_prox_ptr),
	m_storage_ptr(storage_ptr)
{}

// extract_from_socket
std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>
	SlaveGateway::extract_from_socket()
{
	// get request
	std::unique_ptr<DriverProxyBase::DriverData> data_ptr = 
		m_driver_prox_ptr->receive_from_driver();

	// create Args object	
	std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>
		args_ptr(new RequestEngine::RETaskBase::ArgsBase
			(std::move(data_ptr), m_driver_prox_ptr.get(), m_storage_ptr));
	
	size_t key_val = args_ptr->m_data->get_type();
	
	return (std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::
		ArgsBase>>(key_val, std::move(args_ptr)) );
}



}// end of hrd8