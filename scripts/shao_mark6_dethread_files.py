#!/usr/bin/env python

import os
import sys

# See shao_mark6_version.py for more details
_name_ = ' shao_mark6_dethread_files '
_author_ = ' Guo Shaoguang<sgguo@shao.ac.cn> '
_version_ = 'v1.4'

base_bin = '/usr/local/bin/'
files = []


def usage():
    print('[{name}_{version} by {author}]'.format(
        name=_name_, version=_version_, author=_author_))
    print("")
    print("Usage :\t{name} base_dir prefix ".format(name=_name_))
    print("\t > base_dir        : should be */mnt/disks/")
    print("")
    print("For example:")
    print("\t{name} /home/data/     : will dethread all the data in /home/data/".format(name=_name_))
    print("\t{name} /home/data/ abc : will dethread data in /home/data include abc".format(name=_name_))


def get_all_files(data_path):
    # Here to list all the files
    files = os.listdir(data_path)
    return files

def dethread_file(data_path, files, prefix=None):
    print('Dethreading file ... ')
    if prefix is None:
        for i in files:
            os.system(base_bin+'shao_mark6_dqa -d ' + sys.argv[1]+ i)
        #print('gather ' + sys.argv[1] + '/*/*/data/' + i + ' -o ' + infile)
    else:
        for i in files:
            if prefix in i:
                os.system(base_bin+'shao_mark6_dqa -d ' + sys.argv[1]+ i)
                #print('gather ' + base_dir + '/*/*/data/' + i + ' -o ' + outfile)

#def choose_files():

#processing_files = [ i for i in files if sys.argv[3] in i ]

if __name__ == "__main__":
    if len(sys.argv) == 1:
        usage()
    if len(sys.argv) == 2:
        all_files = get_all_files(sys.argv[1])
        dethread_file(sys.argv[1],all_files)
    if len(sys.argv) == 3:
        all_files = get_all_files(sys.argv[1])
        dethread_file(sys.argv[1],all_files,sys.argv[2])
