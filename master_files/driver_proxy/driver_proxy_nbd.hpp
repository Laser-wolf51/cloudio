// -----------------------------------------------------------------------------
// File name  : driver_proxy_nbd.hpp
// Developer  : Eyal Weizman
// Date       : 2019-08-27
// Description: driver_proxy_nbd header
// -----------------------------------------------------------------------------
#ifndef __ILRD_DRIVER_PROXY_NBD_HPP__
#define __ILRD_DRIVER_PROXY_NBD_HPP__

#include "driver_proxy.hpp"

namespace hrd8
{

//===================== Class DriverProxyNBD =================================//
class DriverProxyNBD final : public DriverProxyBase
{
public:
    DriverProxyNBD(const std::string& dev_file, size_t storage_size);
    ~DriverProxyNBD();
    
    // blocks until there is an available request in the socket. when there is -
    // reads the request and returns it via ptr
    std::unique_ptr<DriverData> receive_from_driver();  

    // send a reply to a matcing request 
    void send_to_driver(std::unique_ptr<DriverData> data);
    
    // disconnect gracefully
    void disconnect();
    int get_socket_fd();
    
private:
    int m_socket_fd;
    int m_device_fd;
    int m_nbd_fd;
    bool m_dissconnect;
    std::mutex m_mutex;// making this module MT-safe
    std::mutex m_id_converter_mutex;
    std::thread m_thread;
    std::unordered_map<int, std::string> m_handles_map;
    int m_id_counter;

    static const unsigned int m_reques_magic_number = 0x25609513;
    static const unsigned int m_reply_magic_number = 0x67446698;

    // generic function to test for return values. throws std::runtime_error
    void nbd_thread();
    void read_from_sock(char* buffer, unsigned int count);
    void write_to_sock(const char* buffer, unsigned int count);
    int convert_handle_to_id(const std::string& handle);
    std::string convert_id_to_handle(int req_id);
    size_t ntohll(size_t a);
    
    DriverProxyNBD(const DriverProxyNBD& other) = delete;
    DriverProxyNBD& operator=(const DriverProxyNBD& other) = delete;
    DriverProxyNBD(DriverProxyNBD&& other) = delete;
    DriverProxyNBD& operator=(DriverProxyNBD&& other) = delete;
};

	
}// end of hrd8


#endif // __ILRD_DRIVER_PROXY_NBD_HPP__