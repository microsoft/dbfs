Usage: ./output/bin/dmvtool [Options]
Options:
   -m/--mount-path     :  The mount directory for SQL server(s) DMV files [REQUIRED]
   -c/--conf-file      :  Location of .conf file. [REQUIED]
   -d/--dump-path      :  The dump directory used. Default = "/tmp/sqlserver" [OPTIONAL]
   -v/--verbose        :  Start in verbose mode [OPTIONAL]
   -l/--log-file       :  Path to the log file (only used if in verbose mode) [OPTIONAL]
   -f                  :  Run the DMVTool in foreground [OPTIONAL]
   -h                  :  Print usage


Configuration file needs to be of the following format:
  [server]
  hostname=<>
  username=<>
  password=<>
  version=<>

Example:
  [server]
  hostname=10.120.212.244
  username=sa
  password=Yukon900
  version=16  

Password is optional. If it is not provided for a server entry - user will be prompted for the password.
There can be multiple such entries in the configuration file.

To build the project:
  make

## Note:
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

## Note:
In case of program crash, the fuse module will need to be manually un-mounted.
	- use command:: fusermount -u <mount_directory>
	- The 'mount_directory' should be same as the one passed at program startup.

