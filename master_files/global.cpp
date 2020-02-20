// File Name    :  global.cpp

#include "handleton.hpp"
#include "logger.hpp"
#include "request_engine.hpp"

using namespace hrd8;

INIT_HANDLETON(Logger)

using Factory_req = Factory<RequestEngine::RETaskBase,
							size_t,
							std::unique_ptr<RequestEngine::RETaskBase::ArgsBase>
							>;
INIT_HANDLETON(Factory_req)