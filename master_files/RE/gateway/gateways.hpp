// -----------------------------------------------------------------------------
// File name  : gateways.hpp
// Developer  : Eyal Weizman
// Date       : 2019-08-25
// Description: gateways header
// -----------------------------------------------------------------------------
#ifndef __ILRD_GATEWAYS_HPP__
#define __ILRD_GATEWAYS_HPP__

#include "../../driver_proxy/driver_proxy.hpp"
#include "../../driver_proxy/driver_proxy_nbd.hpp"
#include "../request_engine.hpp"
#include "../../slaves_manager/slaves_manager.hpp"

namespace hrd8
{
//========================== FromNBDGate ===============================//
// FromNBDGate
class FromNBDGate : public RequestEngine::GateWayBase
{
public:
	FromNBDGate(	std::shared_ptr<DriverProxyNBD> nbd_ptr,
					std::shared_ptr<SlavesManager> s_m_ptr);
	~FromNBDGate() = default;
	
	std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>
		extract_from_socket() override;
	
private:
	// allow the future task to deliver the data (Args) to the destination
	std::shared_ptr<DriverProxyNBD> m_nbd_ptr;
	std::shared_ptr<SlavesManager> sm_ptr;
};


//========================== FromCloudGate ===============================//
class FromCloudGate : public RequestEngine::GateWayBase
{
public:
	FromCloudGate(	std::shared_ptr<DriverProxyNBD> nbd_ptr,
					std::shared_ptr<SlavesManager> s_m_ptr,
					int socket_fd,
					size_t slave_index);
	~FromCloudGate() = default;
	
	std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>
		extract_from_socket() override;
	
	int get_fd();
	
	
private:
	// allow the future task to deliver the data (Args) to the destination
	std::shared_ptr<DriverProxyNBD> m_nbd_ptr;
	std::shared_ptr<SlavesManager> sm_ptr;
	int m_socket_fd;
	size_t m_slave_index;
};

	
}// end of hrd8


#endif // __ILRD_GATEWAYS_HPP__