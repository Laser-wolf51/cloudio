// -----------------------------------------------------------------------------
// File name  : plug_and_play_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-23
// Description: plug_and_play test file
// -----------------------------------------------------------------------------
#include <iostream>

#include "plug_and_play.hpp"

using namespace hrd8;

static const std::string g_dir_path = "/home/xyz/git/eyal-waizmann/projects/plugins";

//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n============= plug_and_play test ==============\n" << std::endl;
	try
	{
		DirMonitor dir_monitor(g_dir_path);
		DllLoader dll_loader(dir_monitor.get_dispatcher());
		
		std::cout << "Note: this test will run any .so files in the plugins directory";
		std::cout << "make sure to have in it only fit files." << std::endl;
		
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	catch(const std::exception& e)
	{
		std::cout << "catched!" << std::endl;
		std::cerr << e.what() << '\n';
	}
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}
