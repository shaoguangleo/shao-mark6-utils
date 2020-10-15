#!/bin/bash -
#=========================================================================
#
#          FILE: shao_mount_mark6.sh
#
#         USAGE: ./shao_mount_mark6.sh
#
#=========================================================================

set -o nounset                                  # Treat unset variables as an error

# get all the disks

# get all the disks

aa=`lsscsi| grep -v cd | grep -v sda| awk '{print $6}' `

echo "There are disks in the following:"
echo $aa
index=1
for i in $aa
do
    arr[$index]=$i
    let "index+=1"
done

echo 'There are total $index disks'

echo 'Begin mounting...'

for ((module=1;module<=2;module++))
do
    for ((disk=0;disk<=7;disk++))
    do
        echo 'mount' $module'/'$disk
        let no=(module-1)*8+disk+1
        echo $no
        echo 'mounting ' ${arr[$no]}'1' ' => /mnt/disks/'$module/$disk
        mount ${arr[$no]}1 /mnt/disks/$module/$disk
        echo 'mounting ' ${arr[$no]}'2' ' => /mnt/disks/.meta/'$module/$disk
        mount ${arr[$no]}2 /mnt/disks/.meta/$module/$disk
    done
done
