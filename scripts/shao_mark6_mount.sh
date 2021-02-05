#!/bin/bash -
#=========================================================================
#
#          FILE: shao_mark6_unmount
#
#         USAGE: ./shao_mark6_unmount
#
#=========================================================================

set -o nounset                                  # Treat unset variables as an error

# get all the disks

aa=`lsscsi| grep -v cd | grep -v -w sda| awk '{print $6}' `

echo "There are disks in the following:"
echo $aa
index=1
for i in $aa
do
    arr[$index]=$i
    let "index+=1"
done

let module_no=index/8

echo 'There are total ' $index ' disks, '${module_no} 'module(s) in total'

echo 'Begin mounting...'

for ((module=1;module<=${module_no};module++))
do
    for ((disk=0;disk<=7;disk++))
    do
        let no=(module-1)*8+disk+1
        echo $no ': mount' $module'/'$disk
        echo 'mounting ' ${arr[$no]}'1' ' => /mnt/disks/'$module/$disk
        mount ${arr[$no]}1 /mnt/disks/$module/$disk
        echo 'mounting ' ${arr[$no]}'2' ' => /mnt/disks/.meta/'$module/$disk
        mount ${arr[$no]}2 /mnt/disks/.meta/$module/$disk
    done
done
