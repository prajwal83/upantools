DEPS := $(shell ./depend.sh)
OBJS := $(patsubst %.c,%.o,$(shell find . -name "*.c"))

build: ${OBJS} mosfsutil

include depend.d

mosfsutil: ${OBJS}
	gcc -g -m32 ${OBJS} -o mosfsutil

clean:
	rm -f *.o
	rm -f mosfsutil
