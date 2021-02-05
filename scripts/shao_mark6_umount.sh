#!/bin/bash -
#=========================================================================
#
#          FILE: shao_mark6_mount.sh
#
#         USAGE: ./shao_mark6_mount
#
#=========================================================================

set -o nounset                                  # Treat unset variables as an error

# get all the mount points

mount_points=`mount | grep /mnt/disks/  | awk '{print $3}'`

echo 'Begin ummounting...'
echo $mount_points
for i in $mount_points
do
    echo 'umounting ' $i
    umount $i
done