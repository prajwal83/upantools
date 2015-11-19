obj-m := upanfs.o
upanfs-objs := upanfsmod.o

CFLAGS_upanfs.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
