// -----------------------------------------------------------------------------
// File name  : tasks.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-24
// Description: tasks source file
// -----------------------------------------------------------------------------
#include "driver_proxy.hpp"
#include "handleton.hpp"
#include "request_engine.hpp"
#include "storage.hpp"
#include "tasks.hpp"

namespace hrd8
{
//========================== update_factory ===============================//
// this func will be loaded & run by the DllLoader
// extern "C"
// {
void update_factory()
	{
		// std::cout << "update_factory: load funcs into factory" << std::endl;
		// get a pointer to factory
		
		Factory<RequestEngine::RETaskBase, size_t, std::unique_ptr
		<RequestEngine::RETaskBase::ArgsBase>>* 
		factory_ptr = 
		Handleton<Factory<RequestEngine::RETaskBase,size_t,
		std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>>>::get_instance();
		
		// add all 4 tasks to it
		factory_ptr->add(static_cast<size_t>(DriverProxyBase::DriverData::action_type::READ),
			ReadTask::create_task);
		factory_ptr->add(static_cast<size_t>(DriverProxyBase::DriverData::action_type::WRITE),
			WriteTask::create_task);
		
		std::cout << "update_factory: done" << std::endl;
	}
// }

//========================== ReadTask ===============================//
// create_task
	std::unique_ptr<RequestEngine::RETaskBase> 
	ReadTask::create_task(std::unique_ptr<ArgsBase> task_args)
	{
		return std::unique_ptr<ReadTask>(new ReadTask(std::move(task_args)));
	}

// Ctor
	ReadTask::ReadTask(std::unique_ptr<ArgsBase> task_args) :
	RETaskBase(std::move(task_args))
	{}

// execute
	void ReadTask::execute() noexcept
	{
		try
		{
			std::cout << "ReadTask:: execute" << std::endl;
			
			// prepare buffer size
			get_args()->m_data->get_buffer().resize(get_args()->m_data->get_len());
			
			// read from storage
			auto ptr = get_args()->m_storage->read(std::move(get_args()->m_data));
			
			get_args()->m_driver->send_to_driver(std::move(ptr));
			std::cout << "ReadTask:: reply was sent." << std::endl;
		}
		catch(const std::exception& e)
		{
			std::cerr << "ReadTask: " << e.what() << '\n';
			throw(e);
		}
	}

//========================== WriteTask ===============================//
// create_task
	std::unique_ptr<RequestEngine::RETaskBase> 
	WriteTask::create_task(std::unique_ptr<ArgsBase> task_args)
	{
		return std::unique_ptr<WriteTask>(new WriteTask(std::move(task_args)));
	}

// Ctor
	WriteTask::WriteTask(std::unique_ptr<ArgsBase> task_args) :
	RETaskBase(std::move(task_args))
	{}

// execute
	void WriteTask::execute() noexcept
	{
		try
		{
			std::cout << "WriteTask:: execute" << std::endl;
			
			// take offset before m_data is gone
			size_t offset = get_args()->m_data->get_offset();

			// write to storage
			auto req_ptr = get_args()->m_storage->write(std::move(get_args()->m_data));
		
			// send reply only if its not a backup
			if (offset < get_args()->m_storage->get_slave_size())
			{
				get_args()->m_driver->send_to_driver(std::move(req_ptr));
			}
			// std::cout << "WriteTask:: endddddddddddddd"<< std::endl;
		}
		catch(const std::exception& e)
		{
			std::cerr << "WriteTask: " << '\n';
			throw(e);
		}
	}



}// end of hrd8












// graveyard

// //========================== FlushTask ===============================//
// // create_task
// 	std::unique_ptr<RequestEngine::RETaskBase> 
// 	FlushTask::create_task(std::unique_ptr<ArgsBase> task_args)
// 	{
// 		return std::unique_ptr<FlushTask>(new FlushTask(std::move(task_args)));
// 	}

// // Ctor
// 	FlushTask::FlushTask(std::unique_ptr<ArgsBase> task_args) :
// 	RETaskBase(std::move(task_args))
// 	{}

// // execute
// 	void FlushTask::execute() noexcept
// 	{
// 		std::cout << "FlushTask: execute" << std::endl;
// 		get_args()->m_driver->send_to_driver(std::move(get_args()->get_data()));
// 	}


// //========================== TrimTask ===============================//
// // create_task
// 	std::unique_ptr<RequestEngine::RETaskBase> 
// 	TrimTask::create_task(std::unique_ptr<ArgsBase> task_args)
// 	{
// 		return std::unique_ptr<TrimTask>(new TrimTask(std::move(task_args)));
// 	}

// // Ctor
// 	TrimTask::TrimTask(std::unique_ptr<ArgsBase> task_args) :
// 	RETaskBase(std::move(task_args))
// 	{}

// // execute
// 	void TrimTask::execute() noexcept
// 	{
// 		std::cout << "TrimTask: execute" << std::endl;
// 		get_args()->m_driver->send_to_driver(std::move(get_args()->get_data()));
// 	}
	// void ReadTask::execute() noexcept
	// {
	// 	std::cout << "ReadTask execute. len: " << get_args()->m_data->get_len() << std::endl;
		
	// 	// get pointer to storage
	// 	Storage* storage_ptr = Handleton<Storage>::get_instance();
		
	// 	// get ref to the request's buffer
	// 	auto buffer = get_args()->m_data->get_buffer().data();
				
	// 	storage_ptr->read( buffer,
	// 						get_args()->m_data->get_offset(),
	// 						get_args()->m_data->get_len() );
			
	// 	get_args()->m_driver->send_to_driver(std::move(get_args()->m_data));
	// }