// -----------------------------------------------------------------------------
// File name  : plug_and_play.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-23
// Description: plug_and_play source file
// -----------------------------------------------------------------------------
#include <unistd.h>         // read, close
#include <limits.h>         // NAME_MAX
#include <dlfcn.h>          // dlopen, dlsym, dlclose
#include <dirent.h>         // opendir
#include <sys/inotify.h>    // inotify
#include <errno.h>

#include "../../utils/fail_checker/fail_checker.hpp"
#include "plug_and_play.hpp"

namespace hrd8
{
//======================= static global variables ============================//
static const char* g_shared_obj_func_name = "update_factory";

//========================== DirMonitor ===============================//
// Ctor
	DirMonitor::DirMonitor(const std::string& dir_path) :
		m_dispatcher(),
		m_keep_run(true),
		m_dir_path(dir_path),
		m_sem_files_are_loaded(),
		m_thread(&DirMonitor::thread_func, this)
	{/* std::cout << "DirMonitor: Ctor" << std::endl; */}

// Dtor
	DirMonitor::~DirMonitor()
	{
		// stop the main thread
		m_keep_run = false;
		
		// close the inotify
		try
		{
			check_fail(inotify_rm_watch(m_inot_fd, m_watch_fd), 
				"~DirMonitor: inotify_rm_watch failed");
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		
		// join thread
		m_thread.join();

		std::cout << "DirMonitor::Dtor end" << std::endl;
	}

// get_dispatcher
	Dispatcher<std::string>* DirMonitor::get_dispatcher()
	{
		return (&m_dispatcher);
	}

// thread_func
 void DirMonitor::thread_func()
 {
	try
	{
		// broadcast all existing files
		broadcast_existing_files();

		// init notify
		m_inot_fd = inotify_init();
		check_fail(m_inot_fd, "inotify_init failed");

		// add the directory to the inotify
		m_watch_fd = inotify_add_watch(m_inot_fd, m_dir_path.c_str(), 
			IN_CREATE | IN_MOVE | IN_DELETE | IN_MODIFY );
		check_fail(m_watch_fd, "inotify_add_watch failed");

		// allocate buffer
		struct inotify_event* buf_ptr = static_cast<struct inotify_event*>
			(::operator new(sizeof(struct inotify_event) + NAME_MAX + 1));
		
		size_t bytes_read = 0;
		
		// repeatedly listen to new files in the directory
		while (m_keep_run)
		{
			// read
			bytes_read = read(m_inot_fd, buf_ptr,
				sizeof(struct inotify_event) + NAME_MAX + 1);
			
			// checks if there is what to read
			if (bytes_read > 0)
			{
				if ((buf_ptr->mask == IN_CREATE) | (buf_ptr->mask == IN_MOVED_TO))
				{
					m_dispatcher.broadcast(m_dir_path + "/" + std::string(buf_ptr->name));
					// here - the func DllLoader::load_dll() will be called
				}			
			}
		}
		::operator delete (buf_ptr);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
 }

// broadcast_existing_files
	void DirMonitor::broadcast_existing_files()
	{
		DIR* fdir = opendir(m_dir_path.c_str());
		check_fail(convert_ptr(fdir),
			"DirMonitor::broadcast_existing: opendir failed");
		
		struct dirent *entry;
		entry = readdir(fdir);
		check_fail(convert_ptr(entry),
			"DirMonitor::broadcast_existing: readdir failed");
		std::string name;

		// wait until there is a DLLLoader to broadcast to
		m_sem_dll_is_ready.wait();

		while ( 0 != entry)
		{
		    name = entry->d_name;
			// std::cout << "broadcast_existing: name = " << name << std::endl;
		    if (name != "." && name != "..")
		    {
		        m_dispatcher.broadcast(m_dir_path + "/" + name);
		    }  
		
		    entry = readdir(fdir);
		}
		
		check_fail(closedir(fdir),
			"DirMonitor::broadcast_existing: closedir failed");

		// signaling to the semaphore that all the exsisting files are loaded.
		m_sem_files_are_loaded.post();
	}

// =============================== DllLoader ================================ //
// Ctor
	DllLoader::DllLoader(Dispatcher<std::string>* dispatcher) :
		m_callback( dispatcher,
					*this,
					&DllLoader::load_dll,
					&DllLoader::notify_monitor_out)
	{}

// Dtor
	DllLoader::~DllLoader()
	{
		for(const auto iter : m_handles_vec)
		{
			try
			{
				check_fail(dlclose(iter), "~DllLoader: dlclose failed");
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
				
				// swollow the exception. it is a Dtor after all.
			}
		}
		
		std::cout << "DllLoader: Dtor end" << std::endl;
	}

// load_dll
	void DllLoader::load_dll(const std::string& so_path)
	{
		void (*func)(void) = nullptr;
		
		// load shared object (not the func itself) from dir to memory
		void *dl_handle = dlopen(so_path.c_str(), RTLD_LAZY);
		
		// saves the handlefor future close in the Dtor
		m_handles_vec.push_back(dl_handle);
		
		// get the function from the new shared object & activate it
		func = reinterpret_cast<void (*)(void)>(dlsym(dl_handle, g_shared_obj_func_name));
		func();
	}

// notify_monitor_out
	void DllLoader::notify_monitor_out()
	{
		std::cout << "DllLoader not active anymore" << std::endl;
	}

}// end of hrd8

