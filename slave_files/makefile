#	Filename	:	makefile
#	Developer	:	Eyal Weizman
#	Last Update	:	2019-09-01
#	Description	:	makefile for the slave_files directory

################# vairables #######################
# flags
flags = -std=c++11 -pedantic-errors -Wold-style-cast -Wall -Wextra -g -Wfatal-errors
end_flags = -lconfig++ -pthread -ldl
so_flag = -fPIC -shared

# app
SLAVE_src = slave_main.cpp
SLAVE_out = slave_main.out

# for global.so
global_source = utils/handleton/global.cpp
globals_lib = libglobal.so

# for all_src_lib
sources = \
	driver_proxy/udp_util/udp_util.cpp \
	driver_proxy/driver_data.cpp \
	driver_proxy/driver_proxy_slave.cpp \
	RE/gateway/gateways.cpp \
	RE/pnp/plug_and_play.cpp \
	RE/tasks/tasks.cpp \
	RE/threadpool/thread_pool.cpp \
	RE/request_engine.cpp \
	storage/storage.cpp \
	utils/epoll/epoll.cpp \
	utils/fail_checker/fail_checker.cpp \
	utils/semaphore/semaphore.cpp
	
headers = \
	driver_proxy/udp_util/udp_util.hpp \
	driver_proxy/driver_proxy_slave.hpp \
	driver_proxy/driver_proxy.hpp \
	RE/gateway/gateways.hpp \
	RE/pnp/observer.hpp \
	RE/pnp/plug_and_play.hpp \
	RE/tasks/tasks.hpp \
	RE/threadpool/thread_pool.hpp \
	RE/threadpool/wpq.hpp \
	RE/factory.hpp \
	RE/request_engine.hpp \
	storage/storage.hpp \
	utils/epoll/epoll.hpp \
	utils/fail_checker/fail_checker.hpp \
	utils/handleton/handleton.hpp \
	utils/semaphore/semaphore.hpp \
	utils/scope_lock.hpp
	
all_src_lib = liball_cpp.so


# others
all_libs_for_slave =./$(all_src_lib) ./$(globals_lib)


################ main commands ####################
.PHONY : slave clean

# explaination for slave: since there is a bug with the handleton when calling
# to Storage explicit .so lib (via p&p), only the slave tasks file is linked
# with slave_main.out implicitly, and not via the p&p directory.
slave : $(all_src_lib) $(SLAVE_out)

clean:
	rm -rf *.so *.a *.o *.out *.gch *.out backup_storage plugins/


################ secondary rules ####################

# make shared lib from globals.cpp.
$(globals_lib) : $(global_source)
	g++ $(flags) $(so_flag) $< -o $@

# make a shared library with all the .cpp files
$(all_src_lib) : $(sources) $(globals_lib) $(headers)
	g++ $(flags) $(so_flag) $(sources) ./$(globals_lib) -o $@

# make the app
$(SLAVE_out) : $(SLAVE_src) $(all_libs_for_slave)
	g++ $(flags) $< $(all_libs_for_slave) -o $@ $(end_flags)

