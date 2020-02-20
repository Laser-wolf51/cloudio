#	Filename	:	makefile
#	Developer	:	Eyal Weizman
#	Last Update	:	2019-09-01
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
global_source = global.cpp
globals_lib = libglobal.so

# shared_tasks
tasks_shared_src = tasks.cpp
tasks_shared_so = ./plugins/tasks.so

# for liball_cpp.cpp
filtered = $(MASTER_src) $(tasks_shared_src) $(global_source)
sources = $(filter-out $(filtered), $(wildcard *.cpp))
headers = $(wildcard *.hpp)
objects = $(sources:.cpp=.o)
all_cpp_lib = liball_cpp.so

# others
all_libs_for_master = ./$(globals_lib) ./$(all_cpp_lib)
plugins_dir = ./plugins

################ main commands ####################
.PHONY : master clean

master : $(all_cpp_lib) $(MASTER_out)

clean:
	rm -rf *.so *.a *.o *.out


################ secondary rules ####################
# special rule to make shared lib from global.cpp. global should
# use only this rule.
$(globals_lib) : $(global_source)
	g++ $(flags) $(so_flag) $< -o $@

# make any object file
%.o : %.cpp $(headers)
	g++ $(flags) -fPIC -c $< -o $@

# make a shared library with all the .cpp files
$(all_cpp_lib) : $(objects) $(headers) $(globals_lib)
	g++ $(flags) $(so_flag) $(objects) ./$(globals_lib) -o $@

# making the so (tasks) for the master
$(tasks_shared_so) : $(tasks_shared_src) $(globals_lib) $(all_cpp_lib)
	-mkdir $(plugins_dir)
	g++ $(flags) $(so_flag) $< ./$(globals_lib) ./$(all_cpp_lib) -o $@

# make the app
$(MASTER_out) : $(MASTER_src) $(all_libs_for_master) $(tasks_shared_so)
	g++ $(flags) $< $(all_libs_for_master) -o $@ $(end_flags)



