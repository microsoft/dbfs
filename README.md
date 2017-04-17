# DBFS

DBFS uses FUSE to mount MS SQL Server DMVs as a virtual file system. This gives you the ability to explore information about your database (Dynamic Management Views) using native bash commands!

# Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# Privacy Statement

The [Microsoft Enterprise and Developer Privacy Statement](https://go.microsoft.com/fwlink/?LinkId=786907&lang=en7) describes the privacy statement of this software.

# License

This extension is licensed under the MIT License. Please see the third-party notices file for additional copyright notices and license terms applicable to portions of the software.

# Installation 
1. Download and install all of the follow runtime dependencies: 


2. Download the .deb release package from this Github. 
    
3. Run the following command: 
    dpkg -i release.deb


Check if your installation was successfull by running: 
dbfs -h


# Usage

dbfs -m <mount-path> -c <conf-file-path> [OPTIONS]

Required: 
   -m/--mount-path     :  The mount directory for SQL server(s) DMV files
   -c/--conf-file      :  Location of .conf file.

Options:
   -d/--dump-path      :  The dump directory used. Default = "/tmp/sqlserver"
   -v/--verbose        :  Start in verbose mode
   -l/--log-file       :  Path to the log file (only used if in verbose mode)
   -f                  :  Run DBFS in foreground
   -h                  :  Print usage


Configuration file needs to be of the following format:
  [server]
  hostname=<>
  username=<>
  password=<>
  version=<>

Example:
  [server]
  hostname=00.000.000.000
  username=MyUserName
  password=MyPassword
  version=16 

Password is optional. If it is not provided for a server entry - user will be prompted for the password.
There can be multiple such entries in the configuration file.

# Building 
To build the project:
  make

To build the project - you will need certain packages.
Use the below command to install all of them:
  sudo apt-get install  \
  freetds-dev \
  freetds-bin\
  libunwind-dev \
  fuse \
  libfuse2 \
  libfuse-dev \
  libattr1-dev \
  libavahi-common-dev \
  -y
  
To build the ubuntu package:
  make package-ubuntu

# Issues
Please let us know of any issues you may have by filing an issue on this Github.

In the rare event of a program crash, the fuse module will need to be manually un-mounted.
	- use command:: fusermount -u <mount_directory>
	- The 'mount_directory' should be same as the one passed at program startup.