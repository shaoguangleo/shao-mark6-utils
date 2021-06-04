/**
************************************************************

@file get_file_size.c
@author Shaoguang Guo <sgguo@shao.ac.cn>
@date 2011.11.28
@version 1.0 
@brief This file will get file size
@details details

@copyright Copyright (C),2011-2012
		SHAO,CAS
***************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

unsigned long long int get_file_size(char *file_name)
{
	int fd;
	long long int size;
	struct stat buf;

	fd = open(file_name,O_RDONLY);
	fstat(fd,&buf);
	size = buf.st_size;
	return size;
}
