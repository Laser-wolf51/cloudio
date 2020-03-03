//##############################################################################
//                                                                            //
//      ####    #          ####    #     #   ####      #####    ####          //
//     #    #   #         #    #   #     #   #   #       #     #    #         //
//     #        #         #    #   #     #   #    #      #     #    #         //
//     #        #         #    #   #     #   #    #      #     #    #         //
//     #    #   #         #    #   #     #   #   #       #     #    #         //
//      ####    #######    ####     #####    ####      #####    ####          //
//                                                                            //
//##############################################################################
// -----------------------------------------------------------------------------
// File name  : slave_main.cpp
// Developer  : Eyal Weizman
// Date       : 2019-09-01
// Description: cloudio application
// -----------------------------------------------------------------------------
#include <libconfig.h++>
#include <signal.h>	// sigaction
#include <cstring>	// memset
#include <iostream>

#include "./driver_proxy/driver_proxy.hpp"
#include "./driver_proxy/driver_proxy_slave.hpp"
#include "./RE/request_engine.hpp"
#include "./RE/tasks/tasks.hpp"
#include "./RE/gateway/gateways.hpp"
#include "./storage/storage.hpp"
#include "./utils/handleton/handleton.hpp"
#include "./utils/fail_checker/fail_checker.hpp"

using namespace hrd8;

//========================= internal functions ===============================//
static void sig_handler(int signal);
static void set_signal_behaviour();
// static void update_factory();

static void parse_config_file(
	const std::string& config_file_path,
	std::string& plugins_dir_path,
	std::string& storage_path,
	std::string& master_ip,
	int& port_num,
	int& threads_in_tp,
	size_t& slave_size,
	size_t& message_size);

//====================== global variables & constants ========================//
static RequestEngine* g_req_eng_ptr;
static const std::string CONFIG_FILE_PATH = "slave_config.cfg";


//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n============== cloudio slave ================\n" << std::endl;
	try
	{
		/* define the signal_handler behaviour when pressing C^ */
		set_signal_behaviour();
		
		// prepare outparams for parse_config_file()
		std::string plugins_dir_path;
		std::string storage_path;
		std::string master_ip;
		int port_num(0);
		int threads_in_tp(0);
		size_t slave_size(0);
		size_t message_size(0);
		
		parse_config_file(
			CONFIG_FILE_PATH,
			plugins_dir_path,
			storage_path,
			master_ip,
			port_num,
			threads_in_tp,
			slave_size,
			message_size);
		
		// creates a storage object
		std::shared_ptr<Storage> storage_ptr(new Storage(slave_size, storage_path));
		
		
		// create handletonic factory + load it with task funcs.
		// there is a bug in the handleton. untill it will come to
		// justice - not using p&p
		update_factory();
		
		// create driver proxy
		std::shared_ptr<DriverProxySlave> driver_prox_ptr(new 
		DriverProxySlave(master_ip, port_num, message_size));
		
		// config the RE
		std::shared_ptr<SlaveGateway> gateway_ptr(
			new SlaveGateway(driver_prox_ptr, storage_ptr));
		g_req_eng_ptr = new RequestEngine(plugins_dir_path, threads_in_tp);
		g_req_eng_ptr->register_fd(driver_prox_ptr->get_socket_fd(), gateway_ptr);
		
		// activate the RE
		g_req_eng_ptr->run();
	}
	catch (const std::exception& ex)
	{
		std::cout << "APP: catched! : " << ex.what() << std::endl;
		perror("perror");
	}
	
	delete g_req_eng_ptr;
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}


// ======================= sig_handler =======================================//
void set_signal_behaviour()
{
	struct sigaction sig;
	memset(&sig, 0, sizeof(sig));
	sig.sa_handler = sig_handler;
	check_fail(sigaction(SIGINT, &sig, NULL), "sigaction failed");
}


void sig_handler(int)
{
	g_req_eng_ptr->stop();
}


// ===================== parse_config_file ===================================//
void parse_config_file(
	const std::string& config_file_path,
	std::string& plugins_dir_path,
	std::string& storage_path,
	std::string& master_ip,
	int& port_num,
	int& threads_in_tp,
	size_t& slave_size,
	size_t& message_size)
{
	// create a configuration object
	int outparam;
	bool ret_val;
	libconfig::Config config;
	// load the config file
	config.readFile(config_file_path.c_str());
	
	// start parsing....
	ret_val = config.lookupValue("dir_path", plugins_dir_path);
	check_fail(convert_bool(ret_val), "dir_path not found");
	
	ret_val = config.lookupValue("storage_path", storage_path);
	check_fail(convert_bool(ret_val), "storage_path not found");
	
	ret_val = config.lookupValue("master_ip", master_ip);
	check_fail(convert_bool(ret_val), "master_ip not found");
	
	ret_val = config.lookupValue("port_num", port_num);
	check_fail(convert_bool(ret_val), "port_num not found");
	
	ret_val = config.lookupValue("threads_in_tp", threads_in_tp);
	check_fail(convert_bool(ret_val), "threads_in_tp not found");
	
	ret_val = config.lookupValue("slave_size", outparam);
	slave_size = outparam;
	check_fail(convert_bool(ret_val), "slave_size not found");
	
	config.lookupValue("message_size", outparam);
	message_size = outparam;
	check_fail(convert_bool(ret_val), "message_size not found");
	
	
	// std::cout << "plugins_dir_path = " << plugins_dir_path << std::endl;
	// std::cout << "storage_path = " << storage_path << std::endl;
	// std::cout << "master_ip = " << master_ip << std::endl;
	// std::cout << "port_num = " << port_num << std::endl;
	// std::cout << "threads_in_tp = " << threads_in_tp << std::endl;
	// std::cout << "slave_size = " << slave_size << std::endl;
	// std::cout << "message_size = " << message_size << std::endl;
	
	std::cout << "reading the configuration file: Done" << std::endl;
}

// =========================== graveyard ==================================== //
