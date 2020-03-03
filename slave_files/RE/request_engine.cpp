// -----------------------------------------------------------------------------
// File name  : request_engine.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-06
// Description: request_engine source file
// -----------------------------------------------------------------------------
#include "../utils/handleton/handleton.hpp"
#include "request_engine.hpp"

namespace hrd8
{
//========================== RequestEngine ===================================//
// Ctor
RequestEngine::RequestEngine(
	const std::string& dir_path,
	size_t thread_num,
	std::chrono::milliseconds timeout)
	:
	m_tasks_factory(Handleton<Factory<RETaskBase, size_t,
		std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>>::get_instance()),
	m_thread_pool(thread_num, timeout),
	m_dir_monitor(dir_path),
	m_dll_loader(m_dir_monitor.get_dispatcher()),
	m_epoll(10),
	m_keep_run(true)
{
	// signal that DLL is ready
	m_dir_monitor.m_sem_dll_is_ready.post();

	// block untill the factory is loaded
	m_dir_monitor.m_sem_files_are_loaded.wait();
	
	std::cout << "RE Ctor: done" << std::endl;
}

// Dtor
RequestEngine::~RequestEngine()
{
	m_tasks_factory->clear_all();
}

// process_request
void RequestEngine::process_request(size_t key, std::unique_ptr
	<RequestEngine::RETaskBase::ArgsBase> task_args)
{
	// create task via factory
	auto task_ptr = m_tasks_factory->create(key, std::move(task_args));
	
	// add task to thread pool
	m_thread_pool.add_task(std::move(task_ptr));
}

// add_task_type
void RequestEngine::add_task_type(const DriverProxyBase::DriverData::action_type& key, 
	Factory<RETaskBase, size_t, 
	std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>::CreatorFunc 
	create_func)
{
	m_tasks_factory->add(static_cast<size_t>(key), create_func);
}

// register_fd
void RequestEngine::register_fd(int fd, std::shared_ptr<GateWayBase> gateway_ptr)
{
	m_fds_map.insert({fd, gateway_ptr});
	m_epoll.add(fd, EPOLLIN);
}

// run
void RequestEngine::run()
{
	static const size_t wait_timeout = 9000;

	std::cout << "RE: start main loop" << std::endl;
	
	while (m_keep_run)
	{
		// listen to the fds
		int num_events = m_epoll.wait(wait_timeout);
		std::cout << "\nRE: epoll awake. num_events: " << num_events << std::endl;
		
		for (int i = 0; i < num_events; ++i)
		{
			// get a unique_ptr to Args obj via the match gateway
			int awake_fd = m_epoll[i].fd;
			std::shared_ptr<GateWayBase> gateway_ptr = m_fds_map[awake_fd];
			auto key_and_args = gateway_ptr->extract_from_socket();

			// checks the req type (rellevent only for master)
			if (key_and_args.second->m_data->get_type() != 
				DriverProxyBase::DriverData::action_type::DISC)
			{
				// create a task from it & push to thread pool
				process_request(key_and_args.first, std::move(key_and_args.second));
			}
			else
			{
				// if its a disconnect request - disconnect
				m_keep_run = false;
			}
		}
	}
}

// stop
void RequestEngine::stop()
{
	m_keep_run = false;
}


//========================== RequestEngine::RETaskBase =======================//
// Ctor
RequestEngine::RETaskBase::RETaskBase(std::unique_ptr<ArgsBase> task_args) :
	m_task_args(std::move(task_args))
{}

// get_args
const std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>& RequestEngine::RETaskBase::get_args()
{
	return (m_task_args);
}


//========================== ArgsBase ===============================//
// Ctor
RequestEngine::RETaskBase::ArgsBase::
ArgsBase(std::unique_ptr<DriverProxyBase::DriverData> request,
		DriverProxyBase* driver,
		std::shared_ptr<Storage> storage) 
	:
	m_data(std::move(request)),
	m_driver(driver),
	m_storage(storage)
{}


}// end of hrd8