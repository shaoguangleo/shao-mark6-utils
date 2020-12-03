#!/usr/bin/env python

import os
import sys

_name_ = ' shao_mark6_list_files '
_author_ = ' Guo Shaoguang<sgguo@shao.ac.cn> '
_version_ = 'v1.4'

base_dir = '/mnt/disks/'

def usage():
    print('[{name}_{version} by {author}]'.format(
        name=_name_, version=_version_, author=_author_))
    print("")
    print("Usage :\t{name} base_dir module filename_prefix ".format(
        name=_name_))
    print("\t > base_dir        : should be */mnt/disks/")
    print("\t > module          : which module have the data, should be 1|2|3|4")
    print("\t > filename_prefix : filter data from the prefix")
    print("")
    print("For example:")
    print("\t{name} /mnt/disks/ 1      : will list all the data on module #1".format(name=_name_))
    print("\t{name} /mnt/disks/ 1 test : will list the data include test".format(name=_name_))


def get_all_files(path,module):
    if module is '1':
        files = os.listdir(path + '/1/1/data/')
        return files
    elif module is '2':
        files = os.listdir(path + '/2/1/data/')
        return files
    elif module is '3':
        files = os.listdir(path + '/3/1/data/')
        return files
    elif module is '4':
        files = os.listdir(path + '/4/1/data/')
        return files
    else:
        files = os.listdir(path + '/1/1/data/')
        return files

def get_prefix_files(files,prefix):
    new_files = []
    for i in files:
        if prefix in i:
            new_files.append(i)
    return new_files

if __name__ == "__main__":
    if len(sys.argv) <= 2:
        usage()
    if len(sys.argv) == 3:
        files_on_module = get_all_files(sys.argv[1],sys.argv[2])
        print(('Module[{module}] filelists:').format(module=sys.argv[2]))
        print(files_on_module)
        print(('Module[{module}] include {number} scans').format(module=sys.argv[2], number =len(files_on_module)))
    if len(sys.argv) == 4:
        files_on_module = get_all_files(sys.argv[1],sys.argv[2])
        print(('Module[{module}] filelists:').format(module=sys.argv[2]))
        prefix_files = get_prefix_files(files_on_module, sys.argv[3])
        print(prefix_files)
        print("#"*20)
        print(('Module[{module}] include {number} scans').format(module=sys.argv[2], number =len(files_on_module)))
        print(('Module[{module}] include {number} {prefix} scans').format(module=sys.argv[2], number =len(prefix_files),prefix=sys.argv[3]))
        print("#"*20)