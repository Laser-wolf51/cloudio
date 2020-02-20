# cloudio - a backup application across multiple devices
This is my implementation and design for the final project in InfinityLabs SW development course.
This application is based on a Master-Slave architecture and allows the user to backup files on the 'slave' devices through the master device.

Main features of this app:
*Reliable - every packet of data is back-upped in 2 copies on 2 different slaves (RAID 1), allowing to restore the whole information in case one of the slaves crashes. Plus - each packet is assured to arrive successfully to its destination, otherwise it is resent.
*Fast - not only it uses a local network, it uses it with UDP & thread-pool which execute read/write tasks.
*Cheap - a lot of free storage space already exists in each device.
*Simple to use - all the user should do is just configure the .cfg file and run a script.
*Configurable - this feature allows to add into a special directory a '.so' file with additional functionality while the program is still running.
*When receiving a SIGINT signal (^C) - exit gracefully.

This project is written in C++(std 11) and uses multi-threading, OOD, networking, Bash-scripting, makefiles, design patterns, configuration files, and other tools.
