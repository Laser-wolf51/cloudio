//============================================================================//
// File Name    :	driver_proxy.hpp
// Developer    :	Eyal Weizman
// Date         :	2019-08-22
// Content      :	the Abstract API class for all kinds of DriverProxies,
// 					SlavesManager, DriverProxySlave, DriverProxyNBD,
// 					and also DriverData.
//============================================================================//

// TODO: future option: make slavesmanager part of this family again, and just 
// make receive_from_driver to get an fd. they are conceptually similar after all.

#ifndef __ILRD_DRIVER_PROXY_H__
#define __ILRD_DRIVER_PROXY_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>

#include "udp_util/udp_util.hpp"

namespace hrd8
{
//===================== Class DriverProxyBase ====================================//
// Generic API class for all the DriverProx different classes
class DriverProxyBase // (Abstract)
{
public:
    // holds requests and replys from and to the driver
    class DriverData;
    
    DriverProxyBase() = default;
    virtual ~DriverProxyBase() = default;

    virtual std::unique_ptr<DriverData> receive_from_driver() = 0;
    virtual void send_to_driver(std::unique_ptr<DriverData> data) = 0;
    virtual void disconnect() = 0;
private:
};


//===================== Class DriverData =======================================//
class DriverProxyBase::DriverData 
{
public: 
    
    enum action_type 
    {
        READ,
        WRITE,
        DISC,
        FLUSH,
        TRIM
    };
    
    // Ctor
    DriverData(size_t offset, unsigned int len, int req_id, action_type type);
    
    // API funcs
    size_t get_offset();
	void set_offset(size_t new_offset);
    unsigned int get_len();
    int get_id();
    action_type get_type();
    std::vector<char>& get_buffer();

    // throws std::runtime_error in case of unknown type
    static action_type check_type(unsigned int type);

private:
	size_t m_offset; 
	unsigned int m_len;
    int m_req_id;
	action_type m_type; // enum action_type

    std::vector<char> m_buffer; //data
};

}// end of hrd8


#endif //__ILRD_DRIVER_PROXY_H__