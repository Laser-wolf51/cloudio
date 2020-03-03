// -----------------------------------------------------------------------------
// File name  : tasks.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-22
// Description: tasks header
// -----------------------------------------------------------------------------
#ifndef __ILRD_TASKS_HPP__
#define __ILRD_TASKS_HPP__

#include "../request_engine.hpp"

namespace hrd8
{
// update_factory. for cloudio_slave.cpp
void update_factory();

//========================== ReadTask ========================================//
class ReadTask : public RequestEngine::RETaskBase
{
public:
	static std::unique_ptr<RequestEngine::RETaskBase> 
			create_task(std::unique_ptr<ArgsBase> task_args);
	
	~ReadTask() = default;
	
private:
	ReadTask(std::unique_ptr<ArgsBase> task_args);
	void execute() noexcept;
};


//========================== WriteTask ========================================//
class WriteTask : public RequestEngine::RETaskBase
{
public:
	static std::unique_ptr<RequestEngine::RETaskBase> 
			create_task(std::unique_ptr<ArgsBase> task_args);
	
	~WriteTask() = default;
	
private:
	WriteTask(std::unique_ptr<ArgsBase> task_args);
	void execute() noexcept;
};


}// end of hrd8

#endif // __ILRD_TASKS_HPP__

// ======================== graveyard ========================================//

//========================== FlushTask ========================================//
// class FlushTask : public RequestEngine::RETaskBase
// {
// public:
// 	static std::unique_ptr<RequestEngine::RETaskBase> 
// 			create_task(std::unique_ptr<ArgsBase> task_args);
	
// 	~FlushTask() = default;
	
// private:
// 	FlushTask(std::unique_ptr<ArgsBase> task_args);
// 	void execute() noexcept;
// };


//========================== TrimTask ========================================//
// class TrimTask : public RequestEngine::RETaskBase
// {
// public:
// 	static std::unique_ptr<RequestEngine::RETaskBase> 
// 			create_task(std::unique_ptr<ArgsBase> task_args);
	
// 	~TrimTask() = default;
	
// private:
// 	TrimTask(std::unique_ptr<ArgsBase> task_args);
// 	void execute() noexcept;
// };
