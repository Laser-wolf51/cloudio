// -----------------------------------------------------------------------------
// File name  : gateways.hpp
// Developer  : Eyal Weizman
// Date       : 2019-08-25
// Description: gateways header
// -----------------------------------------------------------------------------
#ifndef __ILRD_GATEWAYS_HPP__
#define __ILRD_GATEWAYS_HPP__

#include "../../driver_proxy/driver_proxy.hpp"
#include "../request_engine.hpp"

namespace hrd8
{
//========================== SlaveGateway ===============================//
class SlaveGateway : public RequestEngine::GateWayBase
{
public:
	SlaveGateway(std::shared_ptr<DriverProxyBase> driver_prox_ptr,
				std::shared_ptr<Storage> storage_ptr);
	~SlaveGateway() = default;
	
	std::pair<size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>
		extract_from_socket()override;

private:
	std::shared_ptr<DriverProxyBase> m_driver_prox_ptr;
	std::shared_ptr<Storage> m_storage_ptr;
};
	
}// end of hrd8


#endif // __ILRD_GATEWAYS_HPP__