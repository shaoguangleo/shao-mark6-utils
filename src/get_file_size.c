/*************************************************************
Copyright (C),2011-2012,SHAO

File name 	: 	get_file_size.c
Author		:	Guo Shaoguang
Version		:	201111
Date		:	2011.11.28
Description	:	This function will create the client
Others		:	Nothing
Function List	:	argument usages
History		:	
	<author>	<time>		<version>	<description>
	Guo Shaoguang	2011.11.28	201111		create the southread_returne code

****************************************************************/
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
