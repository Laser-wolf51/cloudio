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
//========================== SendToCloudTask ================================//
class SendToCloudTask : public RequestEngine::RETaskBase
{
public:
	static std::unique_ptr<RequestEngine::RETaskBase> 
			create_task(std::unique_ptr<ArgsBase> task_args);
	
	~SendToCloudTask() = default;
	
private:
	SendToCloudTask(std::unique_ptr<ArgsBase> task_args);
	void execute() noexcept;
};

//========================== SendToNBDTask ===================================//
class SendToNBDTask : public RequestEngine::RETaskBase
{
public:
	static std::unique_ptr<RequestEngine::RETaskBase> 
			create_task(std::unique_ptr<ArgsBase> task_args);
	
	~SendToNBDTask() = default;
	
private:
	SendToNBDTask(std::unique_ptr<ArgsBase> task_args);
	void execute() noexcept;
};

}// end of hrd8


#endif // __ILRD_TASKS_HPP__