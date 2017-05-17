# DBFS

DBFS uses FUSE to mount MS SQL Server DMVs as a virtual file system. This gives you the ability to explore information about your database (Dynamic Management Views) using native bash commands!

# Examples
<img src="https://github.com/Microsoft/dbfs/raw/master/common/dbfs_demo.gif" alt="demo" style="width:880px;"/>

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

# Installation
Ubuntu:
``` sh
$ curl https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
$ curl https://packages.microsoft.com/config/ubuntu/16.04/prod.list | sudo tee /etc/apt/sources.list.d/msprod.list
$ sudo apt-get update
$ sudo apt-get install mssql-dbfs
```

RHEL:
``` sh
$ sudo su
$ curl https://packages.microsoft.com/config/rhel/7/prod.repo > /etc/yum.repos.d/msprod.repo
$ exit
$ sudo yum update
$ sudo yum install mssql-dbfs
```
 


Check if your installation was successfull by running: 
    `dbfs -h`

# Usage
Setup: 
``` sh
$ dbfs -m <mount-path> -c <conf-file-path> [OPTIONS]
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

# Building
$ Install the following packages:
``` sh
$ sudo apt-get install \
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
$ make
``` 

To build the ubuntu package:
``` sh
$ make package-ubuntu
``` 

To build the rhel7 package:
``` sh
$ make package-rhel7
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
