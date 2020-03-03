// -----------------------------------------------------------------------------
// File name  : udp_util.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-12
// Description: udp_util source file
// -----------------------------------------------------------------------------
#include <unistd.h>
#include <cstring>

#include "../../utils/fail_checker/fail_checker.hpp"
#include "udp_util.hpp"

namespace hrd8
{
//=========================== UdpServer ======================================//
// Ctor
UdpServer::UdpServer(const std::string& addr, int port) :
    m_socket_fd(0),
    m_port(port), 
    m_address(addr),
    m_epoll(1)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", m_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    
    int ret_val = getaddrinfo(addr.c_str(), decimal_port, &hints, &m_addrinfo);
    check_fail(ret_val, "UdpServer Ctor: getaddrinfo failed");
    check_fail(convert_ptr(m_addrinfo), "UdpServer Ctor: getaddrinfo failed");

    m_socket_fd = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    check_fail(m_socket_fd, "UdpServer Ctor: socket failed");

    ret_val = bind(m_socket_fd, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    try
    {
        check_fail(ret_val, "UdpServer Ctor: bind failed");
    }
    catch(const std::exception& e)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket_fd);
        std::cerr << e.what() << '\n';
        throw e;
    }

    // config epoll
    m_epoll.add(m_socket_fd, EPOLLIN);
}

// Dtor
UdpServer::~UdpServer()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket_fd);
}

// receive
int UdpServer::receive(char *buf, size_t max_size)
{
    int flags = 0;
    int address_len;
    address_len = sizeof(m_client_addr);

    int ret_val = recvfrom(m_socket_fd, buf, max_size, flags, 
        reinterpret_cast<struct sockaddr*>(&m_client_addr),
        reinterpret_cast<socklen_t *>(&address_len));
	
    return (ret_val);
}

// timed_receive
int UdpServer::timed_receive(char *buf, size_t max_size, std::chrono::milliseconds timeout_ms)
{
	// std::cout << "UdpServer::timed_receive - start" << std::endl;
	int num_events = m_epoll.wait(timeout_ms.count());
	// std::cout << "UdpServer::timed_receive - num_events = " << num_events << std::endl;

    if (num_events > 0)
    {
        int flags = 0;
        int address_len;
        address_len = sizeof(m_client_addr);
        int ret_val = recvfrom(m_socket_fd, buf, max_size, flags, 
            reinterpret_cast<struct sockaddr*>(&m_client_addr),
            reinterpret_cast<socklen_t *>(&address_len));
        
		// std::cout << "UdpServer::timed_receive - ret_val = " << ret_val << std::endl;
        return (ret_val);
    }
    else
    {
        return (-1);
    }
}

// get_socket
int UdpServer::get_socket() const
{
    return m_socket_fd;
}

// get_port
int UdpServer::get_port() const
{
    return m_port;
}

// get_addr
std::string UdpServer::get_addr() const
{
    return m_address;
}

// send_msg
int UdpServer::send_msg(const char *msg, size_t size)
{
    int flags = 0;
    return sendto(  m_socket_fd, 
                    msg, 
                    size, 
                    flags, 
                    reinterpret_cast<struct sockaddr*>(&m_client_addr),
                    sizeof(m_client_addr));
}

//=========================== UdpClient ======================================//
// Ctor
UdpClient::UdpClient(const std::string& addr, int port) : 
    m_socket_fd(0),
    m_port(port), 
    m_address(addr),
    m_epoll(1)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", m_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int ret_val(getaddrinfo(addr.c_str(), decimal_port, &hints, &m_addrinfo));
    check_fail(ret_val, "invalid address or port for UDP socket");
    check_fail(convert_ptr(m_addrinfo), "invalid address or port for UDP socket");
    
    m_socket_fd = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    try
    {
        check_fail(m_socket_fd, "open socket failed");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        freeaddrinfo(m_addrinfo);
        throw e;
    }

    // config epoll
    m_epoll.add(m_socket_fd, EPOLLIN);
}

// Dtor
UdpClient::~UdpClient()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket_fd);
}

// send_msg
int UdpClient::send_msg(const char *msg, size_t size)
{
    return sendto(m_socket_fd, msg, size, 0, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
}

// get_socket
int UdpClient::get_socket() const
{
    return m_socket_fd;
}

// get_addr
std::string UdpClient::get_addr() const
{
    return m_address;
}

// receive
int UdpClient::receive(char *msg, size_t max_size)
{
    int ret_val = ::recv(m_socket_fd, msg, max_size, 0);
    return ret_val;
}

// timed_receive
int UdpClient::timed_receive(char *msg, size_t max_size, std::chrono::milliseconds timeout_ms)
{
    int num_events = m_epoll.wait(timeout_ms.count());

    if (num_events > 0)
    {
        int ret_val = ::recv(m_socket_fd, msg, max_size, 0);
        return (ret_val);
    }
    else
    {
        return (-1);
    }
}


}// end of hrd8