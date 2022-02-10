#!/vendor/bin/sh

# this script is just for display PQ enhance function
# if the /vendor/productinfo/enhance doesn't exist, copy them from /vendor/etc/enhance

if [ -d /vendor/productinfo/enhance ];then
	echo "directory /vendor/productinfo/enhance already exist, skip copying"
else
	echo "directory /vendor/productinfo/enhance doesn't exist"
	echo "start copy from /vendor/etc/enhance"
	cp /vendor/etc/enhance /vendor/productinfo/ -r
	chmod 664 /vendor/productinfo/enhance/*
fi

