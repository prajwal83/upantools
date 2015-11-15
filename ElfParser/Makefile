DEPS := $(shell ./depend.sh)
OBJS := $(patsubst %.c,%.o,$(shell find . -name "*.c"))

build: ${OBJS} elfparser

include depend.d

elfparser: ${OBJS}
	gcc -g -m32 ${OBJS} -o elfparser

clean:
	rm -f *.o
	rm -f depend.d
	rm -f elfparser
