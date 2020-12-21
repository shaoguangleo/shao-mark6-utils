#!/usr/bin/env python

#TODO check the module automatically

import os
import sys

_name_ = ' shao_mark6_gather_files '
_author_ = ' Guo Shaoguang<sgguo@shao.ac.cn> '
_version_ = 'v1.4'

base_dir = '/mnt/disks/'
data_module_1_have = False
data_module_2_have = False
data_module_3_have = False
data_module_4_have = False
files = []


def usage():
    print('[{name}_{version} by {author}]'.format(
        name=_name_, version=_version_, author=_author_))
    print("")
    print("Usage :\t{name} base_dir module filename_prefix [output_prefix] ".format(name=_name_))
    print("\t > base_dir        : should be */mnt/disks/")
    print("\t > module          : which module have the data")
    print("\t > filename_prefix : filter data from the prefix")
    print("\t > output_prefix   : adding prefix in filename ")
    print("")
    print("For example:")
    print("\t{name} /mnt/disks/ 1234                        : will gather all the data on the disk".format(name=_name_))
    print("\t{name} /mnt/disks/ 1234 shao6                  : will gather the data include shao6 label".format(name=_name_))
    print("\t{name} /mnt/disks/ 1234 shao6_scan29           : will gather the data include shao6 and scan29".format(name=_name_))
    print("\t{name} /mnt/disks/ 1234 shao6_scan29 SheShan   : will add 'SheShan_' prefix before the fielname".format(name=_name_))


def get_all_files():
    if '1' in sys.argv[2]:
        data_module_1_have = True
    if '2' in sys.argv[2]:
        data_module_2_have = True
    if '3' in sys.argv[2]:
        data_module_3_have = True
    if '4' in sys.argv[2]:
        data_module_4_have = True

    # Here to list all the files
    if data_module_1_have:
        files = os.listdir(sys.argv[1]+ '/1/1/data/')
    if data_module_2_have:
        files = os.listdir(sys.argv[1]+ '/2/1/data/')
    if data_module_3_have:
        files = os.listdir(sys.argv[1]+ '/3/1/data/')
    if data_module_4_have:
        files = os.listdir(sys.argv[1]+ '/4/1/data/')

    return files


def gather_file(infile, outfile=None):
    print('Gathering file : ' + infile)
    if outfile is None:
        os.system('shao_mark6_gather ' + sys.argv[1]+ '/*/*/data/' + infile + ' -o ' + infile)
        #print('gather ' + sys.argv[1] + '/*/*/data/' + i + ' -o ' + infile)
    else:
        os.system('shao_mark6_gather ' + sys.argv[1]+ '/*/*/data/' + infile + ' -o ' + outfile)
        #print('gather ' + base_dir + '/*/*/data/' + i + ' -o ' + outfile)

#def choose_files():

#processing_files = [ i for i in files if sys.argv[3] in i ]

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        usage()
    if len(sys.argv) == 3:
        all_files = get_all_files()
        for i in all_files:
            gather_file(i)
    if len(sys.argv) == 4:
        filename_prefix = sys.argv[3]
        all_files = get_all_files()
        for i in all_files:
            if filename_prefix in i:
                gather_file(i)
    if len(sys.argv) == 5:
        filename_prefix = sys.argv[3]
        output_path = sys.argv[4]
        all_files = get_all_files()
        for i in all_files:
            if filename_prefix in i:
                gather_file(i, output_path + '/' + i)
