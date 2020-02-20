// =============================================================================
// File name  : plug_and_play.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-23
// Description: plug_and_play header
// =============================================================================
#ifndef __ILRD_PLUG_AND_PLAY_HPP__
#define __ILRD_PLUG_AND_PLAY_HPP__

#include <string>	// std::string
#include <thread>	// std::thread
#include "semaphore.hpp"
#include "observer.hpp"

namespace hrd8
{

// =============================================================================
// class DirMonitor
// =============================================================================
class DirMonitor final
{
public:
	explicit DirMonitor(const std::string& dir_path);
	~DirMonitor();
	
    Dispatcher<std::string>* get_dispatcher();

	DirMonitor(const DirMonitor& other) = delete;
	DirMonitor(DirMonitor&& other) = delete;
	DirMonitor& operator=(const DirMonitor& other) = delete;
	DirMonitor& operator=(DirMonitor&& other) = delete;
	
private:
	Dispatcher<std::string> m_dispatcher;
    bool m_keep_run;
    const std::string& m_dir_path;
    int m_inot_fd;
    int m_watch_fd;
	Semaphore m_sem_dll_is_ready; // for sync with RE
    Semaphore m_sem_files_are_loaded; // for sync with RE
    std::thread m_thread;
    
	friend class RequestEngine; // so that it can get m_sem_files_are_loaded

	// thread roles:
	// 1. listen to the dir.
	// 2. when .so is added to dir - broadcast to DllLoader new file path.
	// 3. braodcast call to DllLoader notify
	// 4. notify will load the .so. and call the function inside
    void thread_func();
    void broadcast_existing_files();
};

// =============================================================================
// class DllLoader
// =============================================================================
class DllLoader final
{
public:
	explicit DllLoader(Dispatcher<std::string>* dispatcher);
	~DllLoader();

    // anything else?
	
	DllLoader(const DllLoader& otpathher) = delete;
	DllLoader(DllLoader&& other) = delete;
	DllLoader& operator=(const DllLoader& other) = delete;
	DllLoader& operator=(DllLoader&& other) = delete;
	
private:
	Callback<DllLoader, std::string> m_callback;
	std::vector<void*> m_handles_vec;
	
    void load_dll(const std::string& so_path);
    void notify_monitor_out();
};

}// namespace hrd8

#endif // __ILRD_PLUG_AND_PLAY_HPP__
