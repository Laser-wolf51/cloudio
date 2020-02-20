// -----------------------------------------------------------------------------
// File name  : udp_util.hpp
// Developer  : Eyal Weizman
// Date       : 2019-08-12
// Description: udp_util header
// -----------------------------------------------------------------------------
#ifndef __ILRD_UDP_UTIL_HPP__
#define __ILRD_UDP_UTIL_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <chrono>
#include <string>

#include "epoll.hpp"

namespace hrd8
{
//========================== UdpServer ===============================//
class UdpServer
{
public:
    // creates a server and binds it to the given addr & port.
	explicit UdpServer(const std::string& addr, int port);
    ~UdpServer();
    
	// saves the addres of the client
	// returns the num of bytes received or -1 on error.
	int receive(char *buf, size_t max_size);
	
    // saves the addres of the client
	// returns num of bytes, or -1 if timeout passed
    int timed_receive(char *buf, size_t max_size, std::chrono::milliseconds timeout_ms);

	// returns the num of bytes sent or -1 on error.
    int send_msg(const char *msg, size_t size);
    
    int get_socket() const;
    int get_port() const;
    std::string get_addr() const;

private:
    int m_socket_fd;
    int m_port;
    std::string m_address;
    Epoll m_epoll;
    struct addrinfo* m_addrinfo;
    struct sockaddr_in m_client_addr;
};


//========================== UdpClient ===============================//
class UdpClient
{
public:
    explicit UdpClient(const std::string& addr, int port);
    ~UdpClient();

    int send_msg(const char *msg, size_t size);
    int receive(char *msg, size_t max_size);
    int timed_receive(char *msg, size_t max_size, std::chrono::milliseconds timeout_ms);

    int get_socket() const;
    int get_port() const;
    std::string get_addr() const;

private:
    int                 m_socket_fd;
    int                 m_port;
    std::string         m_address;
    Epoll               m_epoll;
    struct addrinfo*    m_addrinfo;
};


}// end of hrd8


#endif // __ILRD_UDP_UTIL_HPP__