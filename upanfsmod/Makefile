obj-m := upanfs.o
upanfs-objs := upanfsmod.o

CFLAGS_upanfsmod.o := -DDEBUG -std=gnu99

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
