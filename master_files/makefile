#	Filename	:	makefile
#	Developer	:	Eyal Weizman
#	Description	:	makefile for the master_files directory

################# vairables #######################
# flags
flags = -std=c++11 -pedantic-errors -Wold-style-cast -Wall -Wextra -g -Wfatal-errors
end_flags = -lconfig++ -pthread -ldl
so_flag = -fPIC -shared

# app
MASTER_src = cloudio_master.cpp
MASTER_out = cloudio_master.out

# for global.so
global_source = utils/handleton/global.cpp
globals_lib = libglobal.so

# shared_tasks
tasks_shared_src = RE/tasks/tasks.cpp
tasks_shared_so = ./plugins/tasks.so

# for all_src_lib
sources = \
	driver_proxy/driver_data.cpp \
	driver_proxy/driver_proxy_nbd.cpp \
	RE/gateway/gateways.cpp \
	RE/pnp/plug_and_play.cpp \
	RE/threadpool/thread_pool.cpp \
	RE/request_engine.cpp \
	slaves_manager/udp_util/udp_util.cpp \
	slaves_manager/slaves_manager.cpp \
	utils/epoll/epoll.cpp \
	utils/fail_checker/fail_checker.cpp \
	utils/semaphore/semaphore.cpp
	
headers = \
	driver_proxy/driver_proxy_nbd.hpp \
	driver_proxy/driver_proxy.hpp \
	RE/gateway/gateways.hpp \
	RE/pnp/observer.hpp \
	RE/pnp/plug_and_play.hpp \
	RE/tasks/tasks.hpp \
	RE/threadpool/thread_pool.hpp \
	RE/threadpool/wpq.hpp \
	RE/factory.hpp \
	RE/request_engine.hpp \
	slaves_manager/udp_util/udp_util.hpp \
	slaves_manager/slaves_manager.hpp \
	utils/epoll/epoll.hpp \
	utils/fail_checker/fail_checker.hpp \
	utils/handleton/handleton.hpp \
	utils/semaphore/semaphore.hpp \
	utils/scope_lock.hpp
	
all_src_lib = liball_cpp.so

# others
all_libs_for_master = ./$(all_src_lib) ./$(globals_lib)
plugins_dir = ./plugins

################ main commands ####################
.PHONY : master clean

master : $(all_src_lib) $(MASTER_out)

clean:
	rm -rf *.so *.a *.o *.gch *.out plugins/


################ secondary rules ####################

# make shared lib from globals.cpp.
$(globals_lib) : $(global_source)
	g++ $(flags) $(so_flag) $< -o $@

# make a shared library with all the .cpp files
$(all_src_lib) : $(sources) $(globals_lib) $(headers)
	g++ $(flags) $(so_flag) $(sources) ./$(globals_lib) -o $@

# making the so (tasks) for the master
$(tasks_shared_so) : $(tasks_shared_src) $(globals_lib) $(all_src_lib)
	-mkdir $(plugins_dir)
	g++ $(flags) $(so_flag) $< ./$(globals_lib) ./$(all_src_lib) -o $@

# make the app
$(MASTER_out) : $(MASTER_src) $(all_libs_for_master) $(tasks_shared_so)
	g++ $(flags) $< $(all_libs_for_master) -o $@ $(end_flags)


