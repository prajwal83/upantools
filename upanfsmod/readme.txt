1. On WSL2, download WSL2 source code from:
2. In checkout directory, run:
  make oldconfig && make prepare
3. Then in upanfsmod directory run:
  make -f Makefile.wsl
====================================

1. To load module: 
  sudo insmod ./upanfs.ko
2. To check if it's loaded
  lsmod | grep upanfs
3. To remove module:
  sudo rmmod upanfs
4. To mount partition to loop device
  sudo kpartx -av <image file path>
5. To mount with upanfs
  sudo mount <mapped loop device> <dir path>
  example: sudo mount /dev/mapper/loop1p1 Mnt/
6. To umount
  sudo umount Mnt/
7. To see messages from modules:
  dmesg
