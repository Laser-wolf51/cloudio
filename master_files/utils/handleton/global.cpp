// File Name    :  global.cpp

#include "handleton.hpp"
#include "../../RE/request_engine.hpp"

using namespace hrd8;

using Factory_req = Factory<RequestEngine::RETaskBase,
							size_t,
							std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>
							>;
INIT_HANDLETON(Factory_req)