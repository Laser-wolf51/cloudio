# Cloudio - a backup application across multiple devices
Cloudio is a backup application, designed to be managed from 1 central device
while saving the files on the storage of multiple other devices (known as
master-slave architecture).  
Including a generic framework which handles most of the app activity.

This project is written in C++(std 11) and uses multi-threading, OOD, 
networking, design patterns, Bash-scripting, makefiles, configuration files, 
NBD, and other tools.

# Main added values & abilities of this app:
Cheap - a lot of free storage space already exists in each device.

Reliable - every packet of data is back-upped in 2 copies on 2 different 
slaves (RAID 1), allowing to restore the whole information in case one of the 
slaves crashes. Plus - each packet is assured to arrive successfully to its 
destination, otherwise it is resent.

Fast - not only it uses a local network, it uses it with UDP & thread-pool 
which execute read/write tasks.

Simple to use - all the user should do is just configure the .cfg file and run
a script.

Runtime configurable - this feature allows to add into a special directory a 
'.so' file with additional functionality while the program is still running.

Generic - the main events-loop takes part inside a generic automated framework
called 'Request Engine', common for both the slave & master programs.

When receiving a SIGINT signal (^C) - exit 'gracefully', while grdually closing
the program's objects using their RAII.

# How to build:
Requirements: Linux OS, download libconfig.

1. in the slave directory - run 'make'.
2. copy to the slave machine the files:
slave_main.out, run.sh, slave_config.cfg, liball_cpp.so, libglobal.so.
3. in the master directory - run 'make'.
4. copy to the master machine the files:
master_main.out, run.sh, master_config.cfg, liball_cpp.so, libglobal.so,
plugins/ .

# How to run:
1. in the configuration files, update the master_ip & define a port for each slave.
2. run the 'run.sh' script.
