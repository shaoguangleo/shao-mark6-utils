# Introduction 

Mark6 utils used by SHAO

## Prerequisites

You should install aio for asynchronous I/O access library, type the following command.

On CentOS

```bash
$ sudo yum install libaio libaio-devel
```
On Ubuntu
```bash
$ sudo apt-cache search aio
$ sudo apt-get install libaio1 libaio-dev
```

## Installation

Just type the following command to install all the utils

```bash
$ git clone https://github.com/shaoguangleo/shao-mark6-utils.git
$ cd shao-mark6-utils
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr/local/bin/ .. 
$ make 
$ make install
```

## Document

Just type `doxygen Doxyfile`, you will get html and latex document for this repo.

# Version

- M6Utils : 1.0.7
- d-plane : 1.17
- scripts : 1.5.0
