#!/sbin/busybox sh
 
echo "do sensors job"
chown root:input /sys/devices/virtual/input/input*/*enable*
chown root:input /sys/devices/virtual/input/input*/*delay*
