// -----------------------------------------------------------------------------
// File name  : request_engine.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-22
// Description: request_engine header
// -----------------------------------------------------------------------------
#ifndef __REQUEST_ENGINE_HPP__
#define __REQUEST_ENGINE_HPP__

#include "epoll.hpp"
#include "driver_proxy.hpp"
#include "factory.hpp"
#include "thread_pool.hpp"
#include "plug_and_play.hpp"
#include "storage.hpp" // needed by the slave via Args

namespace hrd8
{
// =============================================================================
//                               class RequestEngine                            
// =============================================================================
class RequestEngine
{
public:
    class GateWayBase;

    // class RETaskBase:
    // the tasks that will be produced in the factory.
    class RETaskBase : public ThreadPool::Task
    {
    public: 
        class ArgsBase;
        
        RETaskBase(std::unique_ptr<ArgsBase> task_args);
        virtual ~RETaskBase() = default;
        
        // must be implemented by the derived class!
        // factory will use this func to create tasks
        static std::unique_ptr<RequestEngine::RETaskBase> 
            create_task(std::unique_ptr<ArgsBase> task_args);
        
        const std::unique_ptr<ArgsBase>& get_args();
    
    private:
        std::unique_ptr<ArgsBase> m_task_args;
        
        // thread execution func
        void execute() noexcept override = 0;
        friend class ThreadPool;
    };
    
    // RequestEngine special members
    RequestEngine(const std::string& dir_path,
        size_t thread_num = std::thread::hardware_concurrency(),
        std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
    ~RequestEngine();
    
    // adds to the factory another creating-tasks function
    void add_task_type(const DriverProxyBase::DriverData::action_type& key, 
    Factory<RETaskBase, size_t, std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>::CreatorFunc 
    create_func);
    
    // make factory create a task, according to the request type inside ArgsBase
    // as a key, then adds the task to the thread pool.
    void process_request(size_t key,
                std::unique_ptr<RequestEngine::RETaskBase::ArgsBase> task_args);
    
    // adds a fd to monitor, and attatch a gateway object to it
    void register_fd(int fd, std::shared_ptr<GateWayBase> gateway_ptr);
    
    // start listening to all the registered sockets, getting requests from them,
    // create tasks from those requests & execute via thread-pool.
    void run();
    
    // stop listening
    void stop();
    

private:
    //singleton factory
    Factory<RETaskBase, 
            size_t, 
            std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>* m_tasks_factory;
    
    ThreadPool m_thread_pool;
    DirMonitor m_dir_monitor;
    DllLoader m_dll_loader;
    Epoll m_epoll;
    std::unordered_map<int, std::shared_ptr<GateWayBase>> m_fds_map;
    bool m_keep_run; // can be modified via this object
};


// =============================================================================
//                               class ArgsBase
// =============================================================================
class RequestEngine::RETaskBase::ArgsBase
{
public:
	// ArgsBase(std::unique_ptr<DriverProxyBase::DriverData> request, 
	// 			DriverProxyBase* driver, std::shared_ptr<Storage> storage = nullptr);
	ArgsBase(std::unique_ptr<DriverProxyBase::DriverData> request, 
				DriverProxyBase* driver,
				std::shared_ptr<Storage> storage);
	~ArgsBase() = default;

	// std::unique_ptr<DriverProxyBase::DriverData>& get_data();

    std::unique_ptr<DriverProxyBase::DriverData> m_data;
	DriverProxyBase* m_driver;
	std::shared_ptr<Storage> m_storage;
	
private:

};

// =============================================================================
//                               class GateWayBase
// =============================================================================
class RequestEngine::GateWayBase
{
public:
    GateWayBase() = default;
    virtual ~GateWayBase() = default;

    // function role: extract from socket a request and return the info wrraped
    // by ArgsBase.
    virtual std::pair<size_t, std::unique_ptr<RETaskBase::ArgsBase>>
		extract_from_socket() = 0;
private:
};

} // namespace hrd8

#endif // __REQUEST_ENGINE_HPP__
