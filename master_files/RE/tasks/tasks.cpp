// -----------------------------------------------------------------------------
// File name  : tasks.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-24
// Description: tasks source file
// -----------------------------------------------------------------------------
#include "../../driver_proxy/driver_proxy.hpp"
#include "../../utils/handleton/handleton.hpp"
#include "../request_engine.hpp"
#include "tasks.hpp"

namespace hrd8
{
//========================== update_factory ===============================//
// this func will be loaded & run by the DllLoader
extern "C"
{
	void update_factory()
	{
		std::cout << "update_factory: load funcs into factory" << std::endl;
		// get a pointer to factory
		Factory<RequestEngine::RETaskBase, size_t, std::unique_ptr
		<RequestEngine::RETaskBase::ArgsBase>>* 
		factory_ptr = 
		Handleton<Factory<RequestEngine::RETaskBase,size_t,
		std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>>::get_instance();
		
		size_t to_cloud_key = SlavesManager::tasks_keys::TO_CLOUD_TASK;
		size_t to_nbd_key = SlavesManager::tasks_keys::TO_NBD_TASK;
		
		// add tasks to factory
		factory_ptr->add(to_cloud_key, &SendToCloudTask::create_task);
		factory_ptr->add(to_nbd_key, &SendToNBDTask::create_task);
		
		// std::cout << "update_factory: done" << std::endl;
	}
}


//========================== SendToCloudTask ===============================//
// create_task
	std::unique_ptr<RequestEngine::RETaskBase> 
	SendToCloudTask::create_task(std::unique_ptr<ArgsBase> task_args)
	{
		// std::cout << "SendToCloudTask::create_task" << std::endl;
		return std::unique_ptr<SendToCloudTask>(new SendToCloudTask(
			std::move(task_args)));
	}

// Ctor
	SendToCloudTask::SendToCloudTask(std::unique_ptr<ArgsBase> task_args) :
	RETaskBase(std::move(task_args))
	{/* std::cout << "SendToCloudTask::Ctor" << std::endl; */}

// execute
	void SendToCloudTask::execute() noexcept
	{
		try
		{
			std::cout << "SendToCloudTask::execute" << std::endl;
			
			// send data to the cloud
			get_args()->m_sm_ptr->send_to_driver(std::move(get_args()->m_data));
			std::cout << "SendToCloudTask: Done" << std::endl;
		}
		catch(const std::exception& e)
		{
			std::cerr << "SendToCloudTask: " << e.what() << '\n';
			// throw(e);
		}
	}

//========================== SendToNBDTask ===================================//
// create_task
	std::unique_ptr<RequestEngine::RETaskBase> 
	SendToNBDTask::create_task(std::unique_ptr<ArgsBase> task_args)
	{
		// std::cout << "SendToNBDTask::create_task" << std::endl;
		return std::unique_ptr<SendToNBDTask>(new SendToNBDTask(std::move(task_args)));
	}

// Ctor
	SendToNBDTask::SendToNBDTask(std::unique_ptr<ArgsBase> task_args) :
	RETaskBase(std::move(task_args))
	{/* std::cout << "SendToNBDTask::Ctor" << std::endl; */}

// execute
	void SendToNBDTask::execute() noexcept
	{
		try
		{
			std::cout << "SendToNBDTask::execute" << std::endl;
			
			// send data to the cloud
			get_args()->m_driver->send_to_driver(std::move(get_args()->m_data));
			std::cout << "SendToNBDTask: Done" << std::endl;
		}
		catch(const std::exception& e)
		{
			std::cerr << "SendToNBDTask: " << e.what() << '\n';
			// throw(e); TODO:
		}
	}


}// end of hrd8


// graveyard