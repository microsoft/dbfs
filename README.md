# DBFS
<img src="https://travis-ci.org/Microsoft/dbfs.svg?branch=master" alt="master_build_status" style="width:800px;"/>

DBFS uses FUSE to mount MS SQL Server DMVs as a virtual file system. This gives you the ability to explore information about your database (Dynamic Management Views) using native bash commands!


# Installation
Ubuntu:
``` sh
sudo wget https://github.com/Microsoft/dbfs/releases/download/0.1.5/dbfs_0.1.5_amd64.deb
sudo dpkg -i dbfs_0.1.5_amd64.deb
sudo apt-get install -f
```

RHEL:
``` sh
sudo wget https://github.com/Microsoft/dbfs/releases/download/0.1.5/dbfs-0.1.5-0.x86_64.rpm
sudo wget https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
sudo rpm -ivh epel-release-latest-7.noarch.rpm
sudo yum update
sudo yum install dbfs-0.1.5-0.x86_64.rpm
```

Check if your installation was successfull by running: 
    `dbfs -h`

Note: DBFS for SUSE linux and apt-get/yum package installs for Ubuntu/Red Hat coming soon!

# Quick Start 
Change directory to a directory where you want to create your config file and mounting directory. Example:
``` sh
$ cd ~/demo
``` 

Create a directory you want the DMVs to mount to
``` 
$ mkdir dmv
``` 
 
Create a file to store the configuration
``` 
touch dmvtool.config
``` 
 
Edit the config file using an editor like VI
``` 
vi dmvtool.config
``` 
The contents of the file should be
``` 
[server friendly name]
hostname=[HOSTNAME]
username=[DATBASE_LOGIN]
password=[PASSWORD]
version=[VERSION]
``` 
Example:\
[server]\
hostname=00.000.000.000\
username=MyUserName\
password=MyPassword\
version=16

Run the tool
```
dbfs -c ./[Config File] -m ./[Mount Directory]
```
 
Example
```
dbfs -c ./dmvtool.config -m ./dmv
```
 
See DMV in the directory
```
cd dmv
```
 
You should see the list of your friendly server names by running 'ls'
```
cd <server friendly name>
```
 
You should see the list of DMVs as files by running 'ls'. To Look at the contents of one of the files:
```
more <dmv file name>
```
 
You can pipe the output from DMVTool to tools like cut (CSV) and jq (JSON) to format the data for better readability.
 
By default, DBFS runs in background. You can shut it down using the following commands:
```
ps -A | grep dbfs kill -2 <dmvtool pid>
```
If you want to run it in the foreground you can pass the -f parameter. You can pass the -v parameter for verbose output if you are running the tool in the foreground.

# Usage
Setup: 
``` sh
dbfs -m <mount-path> -c <conf-file-path> [OPTIONS]
```

Required:\
    -m/--mount-path     :  The mount directory for SQL server(s) DMV files\
    -c/--conf-file      :  Location of .conf file.\
    
Optional:\
    -d/--dump-path      :  The dump directory used. Default = "/tmp/sqlserver"\
    -v/--verbose        :  Start in verbose mode\
    -l/--log-file       :  Path to the log file (only used if in verbose mode)\
    -f                  :  Run DBFS in foreground\
    -h                  :  Print usage
    
Configuration file needs to be of the following format:\
[server]\
hostname=<>\
username=<>\
password=<>\
version=<>

Example:\
[server]\
hostname=00.000.000.000\
username=MyUserName\
password=MyPassword\
version=16

The password is optional. If it is not provided for a server entry - user will be prompted for the password.
There can be multiple such entries in the configuration file.

# Examples
<img src="https://github.com/Microsoft/dbfs/raw/master/common/dbfs_demo.gif" alt="demo" style="width:800px;"/>

Demo Commands:
``` sh
$ dbfs -m ~/demo/mount -c ~/demo/local_server.conf 
$ cd ~/demo/mount/local_server
$ ls
$ ls | grep -i os | grep -i memory
$ cat dm_os_sys_memory
$ cat dm_os_sys_memory.json
$ cat dm_os_sys_memory.json | python -m json.tool
$ awk '{print $1,$5}' dm_os_sys_memory | column -t
$ join -j 1 -o 1.1,1.16,1.17,2.5,2.8 <(sort -k1 dm_exec_connections) <(sort -k1 dm_exec_connections) -t $'\t' | sort -n -k1 | column -t
```

# Building
 Install the following packages:
``` sh
 sudo apt-get install \
  	freetds-dev \
  	freetds-bin \
  	libunwind-dev \
  	fuse \
  	libfuse2 \
  	libfuse-dev \
  	libattr1-dev \
  	libavahi-common-dev \
  	-y
```

To build the project:
``` sh
 make
``` 

To build the ubuntu package:
``` sh
 make package-ubuntu
``` 

To build the rhel7 package:
``` sh
 make package-rhel7
``` 

# Issues
Please let us know of any issues you may have by filing an issue on this Github.

In the rare event of a program crash, the fuse module will need to be manually un-mounted.\
	- use command: `fusermount -u <mount_directory>`\
	- The 'mount_directory' should be same as the one passed at program startup.
	
# Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# Privacy Statement

The [Microsoft Enterprise and Developer Privacy Statement](https://go.microsoft.com/fwlink/?LinkId=786907&lang=en7) describes the privacy statement of this software.

# License

This extension is licensed under the MIT License. Please see the third-party notices file for additional copyright notices and license terms applicable to portions of the software.
 
