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
// File name  : main_master.cpp
// Developer  : Eyal Weizman
// Date       : 2019-08-14
// Description: cloudio Master application
// -----------------------------------------------------------------------------
#include <libconfig.h++>
#include <signal.h>	// sigaction
#include <cstring>	// memset
#include <iostream>

#include "utils/fail_checker/fail_checker.hpp"
#include "driver_proxy/driver_proxy.hpp"
#include "driver_proxy/driver_proxy_nbd.hpp"
#include "RE/gateway/gateways.hpp"
#include "RE/request_engine.hpp"
#include "slaves_manager/slaves_manager.hpp"

using namespace hrd8;

//========================= internal functions ===============================//
static void sig_handler(int signal);
static void set_signal_behaviour();

static void parse_config_file(
	const std::string& config_file_path,
	std::string& plugins_dir_path,
	std::string& this_ip,
	int& threads_in_tp,
	size_t& slave_size,
	size_t& message_size,
	std::chrono::seconds& wait_for_slave_timeout,
	std::chrono::seconds& timeout_before_resend,
	int& num_of_slaves,
	std::vector<int>& ports_vec);

static void connect_to_slaves(
	std::shared_ptr<SlavesManager> slave_manager_ptr,
	std::vector<int>& ports_vec, 
	std::vector<std::pair<int, size_t>>& ret_pairs_vec);

static void config_RE(
	std::shared_ptr<DriverProxyNBD> nbd_proxy_ptr, 
	std::shared_ptr<SlavesManager> slave_manager_ptr, 
	std::vector<std::pair<int, size_t>>& ret_pairs_vec);

//====================== global variables & constants ========================//
static std::shared_ptr<RequestEngine> g_req_eng_ptr;
static const std::string CONFIG_FILE_PATH = "master_config.cfg";

//=========================== MAIN ===========================================//
int main(int, char* argv[])
{
	std::cout << "\n============== cloudio master ================\n" << std::endl;
	try
	{
		// define the signal_handler behaviour when pressing C^
		set_signal_behaviour();
		
		// prepare outparams for parse_config_file()
		std::string plugins_dir_path;
		std::string this_ip;
		int threads_in_tp(0);
		size_t slave_size(0);
		size_t message_size(0);
		std::chrono::seconds wait_for_slave_timeout(0);
		std::chrono::seconds timeout_before_resend(0);
		int num_of_slaves(0);
		std::vector<int> ports_vec;
		
		parse_config_file(
			CONFIG_FILE_PATH,
			plugins_dir_path,
			this_ip,
			threads_in_tp,
			slave_size,
			message_size,
			wait_for_slave_timeout,
			timeout_before_resend,
			num_of_slaves,
			ports_vec);

		// create slave manager
		std::shared_ptr<SlavesManager> slave_manager_ptr(new SlavesManager(
			this_ip,
			slave_size,
			num_of_slaves,
			message_size,
			wait_for_slave_timeout,
			timeout_before_resend)
			);
		
		// connect to slaves
		std::vector<std::pair<int, size_t>> ret_pairs_vec;
		connect_to_slaves(slave_manager_ptr, ports_vec, ret_pairs_vec);
		
		// create NBD driver proxy
		size_t storage_size = num_of_slaves * slave_size;
		const std::string dev_name(argv[1]);
		std::shared_ptr<DriverProxyNBD> nbd_proxy_ptr(new 
			DriverProxyNBD(dev_name, storage_size));
		
		// create RE
		g_req_eng_ptr = std::make_shared<RequestEngine>(plugins_dir_path, threads_in_tp);
		
		// config RE
		config_RE(nbd_proxy_ptr, slave_manager_ptr, ret_pairs_vec);
		
		// **** LET THE APP BEGIN! **** //
		g_req_eng_ptr->run();
		
	}
	catch (const std::exception& ex)
	{
		std::cout << "APP: catched! : " << ex.what() << std::endl;
		perror("perror");
	}
	
	g_req_eng_ptr = nullptr;

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
	// might be NULL when C^ was presssed before creating the RE
	if (g_req_eng_ptr != nullptr)
	{
		g_req_eng_ptr->stop();
	}
}


// ===================== parse_config_file ===================================//
void parse_config_file(
	const std::string& config_file_path,
	std::string& plugins_dir_path,
	std::string& this_ip,
	int& threads_in_tp,
	size_t& slave_size,
	size_t& message_size,
	std::chrono::seconds& wait_for_slave_timeout,
	std::chrono::seconds& timeout_before_resend,
	int& num_of_slaves,
	std::vector<int>& ports_vec)
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
	
	ret_val = config.lookupValue("threads_in_tp", threads_in_tp);
	check_fail(convert_bool(ret_val), "threads_in_tp not found");
	
	ret_val = config.lookupValue("this_ip", this_ip);
	check_fail(convert_bool(ret_val), "this_ip not found");
	
	ret_val = config.lookupValue("slave_size", outparam);
	slave_size = outparam;
	check_fail(convert_bool(ret_val), "slave_size not found");
	
	config.lookupValue("message_size", outparam);
	message_size = outparam;
	check_fail(convert_bool(ret_val), "message_size not found");
	
	config.lookupValue("wait_for_slave_timeout", outparam);
	wait_for_slave_timeout = std::chrono::seconds(outparam);
	check_fail(convert_bool(ret_val), "wait_for_slave_timeout not found");
	
	config.lookupValue("timeout_before_resend", outparam);
	timeout_before_resend = std::chrono::seconds(outparam);
	check_fail(convert_bool(ret_val), "timeout_before_resend not found");
	
	// get num_of_slaves and load all the ports in the vector
	const libconfig::Setting &ports_arr_cfg = config.getRoot()["ports_arr"];
	num_of_slaves = ports_arr_cfg.getLength();
	for(int i = 0; i < num_of_slaves; ++i)
	{
		const libconfig::Setting &port_num_cfg = ports_arr_cfg[i];
		int port_num = port_num_cfg;
		ports_vec.push_back(port_num);
		// std::cout << "port_num = " << port_num << std::endl;
	}
	
	// std::cout << "plugins_dir_path = " << plugins_dir_path << std::endl;
	// std::cout << "threads_in_tp = " << threads_in_tp << std::endl;
	// std::cout << "this_ip = " << this_ip << std::endl;
	// std::cout << "slave_size = " << slave_size << std::endl;
	// std::cout << "message_size = " << message_size << std::endl;
	// std::cout << "num_of_slaves = " << num_of_slaves << std::endl;
	
	std::cout << "reading the configuration file: Done" << std::endl;
}

// ======================= connect_to_slaves =================================//
void connect_to_slaves(
	std::shared_ptr<SlavesManager> slave_manager_ptr,
	std::vector<int>& ports_vec, 
	std::vector<std::pair<int, size_t>>& ret_pairs_vec)
{
	// adding the ports to S.M.
	for (auto port_num : ports_vec)
	{
		// create connection with the slave
		auto slave_properites = slave_manager_ptr->add_slave(port_num);
		
		// push the returned pair to a vector for later
		ret_pairs_vec.push_back(slave_properites);
	}
}

// ======================= config_RE =========================================//
void config_RE(
	std::shared_ptr<DriverProxyNBD> nbd_proxy_ptr, 
	std::shared_ptr<SlavesManager> slave_manager_ptr, 
	std::vector<std::pair<int, size_t>>& ret_pairs_vec)
{
	// create a vector of FromCloudGate ptrs
	std::vector<std::shared_ptr<FromCloudGate>> gateways_vec;
	for(auto fd_and_index : ret_pairs_vec)
	{
		// create gateway from cloud to NBD
		std::shared_ptr<FromCloudGate> from_cloud_gateway(new 
			FromCloudGate(	nbd_proxy_ptr,
							slave_manager_ptr,
							fd_and_index.first,
							fd_and_index.second));
		
		// push to vec
		gateways_vec.push_back(from_cloud_gateway);
	}
	
	// create gateway from NBD to cloud
	std::shared_ptr<FromNBDGate> from_nbd_gateway(new 
	FromNBDGate(nbd_proxy_ptr, slave_manager_ptr));
	
	// config the RE
	g_req_eng_ptr->register_fd(nbd_proxy_ptr->get_socket_fd(), from_nbd_gateway);
	
	for(auto from_cloud_gateway : gateways_vec)
	{
		g_req_eng_ptr->register_fd(from_cloud_gateway->get_fd(), from_cloud_gateway);
	}
}

// =========================== graveyard ==================================== //

