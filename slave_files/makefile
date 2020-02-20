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
SLAVE_src = cloudio_slave.cpp
SLAVE_out = cloudio_slave.out

# for global.so
global_source = global.cpp
globals_lib = libglobal.so

# for liball_cpp.cpp
filtered = $(SLAVE_src) $(global_source)
sources = $(filter-out $(filtered), $(wildcard *.cpp))
headers = $(wildcard *.hpp)
objects = $(sources:.cpp=.o)
all_cpp_lib = liball_cpp.so

# others
all_libs_for_slave =./$(globals_lib) ./$(all_cpp_lib)

################ main commands ####################
.PHONY : slave clean

# explaination for slave: since there is a bug with the handleton when calling
# to Storage explicit .so lib (via p&p), only the slave tasks file is linked
# with cloudio_slave.out implicitly, and not via the p&p directory.
slave : $(all_cpp_lib) $(SLAVE_out)

clean:
	rm -f *.so *.a *.o *.out


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


$(SLAVE_out) : $(SLAVE_src) $(all_libs_for_slave)
	g++ $(flags) $< $(all_libs_for_slave) -o $@ $(end_flags)

