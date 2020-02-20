// -----------------------------------------------------------------------------
// File name  : driver_proxy_slave.hpp
// Developer  : Eyal Weizman
// Date       : 2019-08-27
// Description: driver_proxy_slave header
// -----------------------------------------------------------------------------
#ifndef __ILRD_DRIVER_PROXY_SLAVE_HPP__
#define __ILRD_DRIVER_PROXY_SLAVE_HPP__

#include "driver_proxy.hpp"

namespace hrd8
{

//===================== Class DriverProxySlave ====================================//
class DriverProxySlave : public DriverProxyBase
{
public:
	DriverProxySlave(const std::string& server_ip, int port_num, size_t message_size);
	~DriverProxySlave();

	// blocks until there is an available request in the socket. when there is -
	// reads the request and returns it via ptr
	std::unique_ptr<DriverData> receive_from_driver() override;  

	// send a reply to a matcing request 
	void send_to_driver(std::unique_ptr<DriverData> data) override;

	// disconnect gracefully
	void disconnect() override;
	int get_socket_fd();

private:
	const std::string m_server_ip;
	const int m_port;
	const size_t m_message_size;
	UdpClient m_master_sock;
	
	DriverProxySlave(const DriverProxySlave& other) = delete;
	DriverProxySlave& operator=(const DriverProxySlave& other) = delete;
	DriverProxySlave(DriverProxySlave&& other) = delete;
	DriverProxySlave& operator=(DriverProxySlave&& other) = delete;
};
	
}// end of hrd8


#endif // __ILRD_DRIVER_PROXY_SLAVE_HPP__