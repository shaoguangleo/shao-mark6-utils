# Script Programs

- Data Check :  shao_mark6_list_files
- Data Gather : shao_mark6_gather_files
- Data Dethread: shao_mark6_dethread_files
- Data Delete :  shao_mark6_delete_files
- Data Analysis:  shao_mark6_analysis_files

## Data Check

```bash
$ shao_mark6_list_files_v1.4.4
[ shao_mark6_list_files _ v0.1  by  Guo Shaoguang<sgguo@shao.ac.cn> ]

Usage :	 shao_mark6_list_files  base_dir module filename_prefix
	 > base_dir        : should be */mnt/disks/
	 > module          : which module have the data, should be 1|2|3|4
	 > filename_prefix : filter data from the prefix

For example:
	 shao_mark6_list_files  /mnt/disks/ 1      : will list all the data on module #1
	 shao_mark6_list_files  /mnt/disks/ 1 test : will list the data include test
```

## Data Gather

```bash
$ shao_mark6_gather_files 1234 data_prefix output_prefix

# This script can gather one file/one scan or one IDCODE in one step.

Usage :	 shao_mark6_gather_files  module filename_prefix [output_prefix]
	 > module          : which module have the data
	 > filename_prefix : filter data from the prefix
	 > output_prefix   : adding prefix in filename

For example:
	 shao_mark6_gather_files  1234                        : will gather all the data on the disk
	 shao_mark6_gather_files  1234 shao6                  : will gather the data include shao6 label
	 shao_mark6_gather_files  1234 shao6_scan29           : will _gather the data include shao6 and scan29
	 shao_mark6_gather_files  1234 shao6_scan29 SheShan   : will add 'SheShan_' prefix before the fielname
```

## Data Dethread

```bash
$ shao_mark6_dethread_files.py 

Usage :  shao_mark6_dethread_files  base_dir prefix 
         > base_dir        : should be */mnt/disks/

For example:
         shao_mark6_dethread_files  /home/data/     : will dethread all the data in /home/data/
         shao_mark6_dethread_files  /home/data/ abc : will dethread data in /home/data include abc
```

## Mount the Mark6 manually

It happeds a lot that we can not mount disks on Mark6, if we just want to playback the data, using the following command to mount manually

```bash
$ sudo shao_mark6_mount
```