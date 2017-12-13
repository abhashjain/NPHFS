fusermount -u ~/mnt
rm -rf ~/mnt
sudo rmmod npheap

sudo insmod npheap.ko
sudo chmod 777 /dev/npheap
mkdir ~/mnt
./configure
sudo make
sudo make install
./src/nphfuse -d /dev/npheap ~/mnt
