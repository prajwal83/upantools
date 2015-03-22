CC=gcc

CFLAGS = -g

ELF_HEADERS = ElfConstants.h \
			ElfHacker.h \
			ElfHeader.h \
			ElfProgHeader.h \
			ElfSectionHeader.h \
			ElfSymbolTable.h \
			ElfDynamicSection.h \
			ElfRelocationSection.h \
			ElfHashTable.h \
			ElfStringTable.h

ELF_SRC = ElfHackerMain.c \
			ElfHacker.c \
			ElfHeader.c \
			ElfProgHeader.c \
			ElfSectionHeader.c \
			ElfSymbolTable.c \
			ElfDynSymbolTable.c \
			ElfDynamicSection.c \
			ElfRelocationSection.c \
			ElfHashTable.c \
			ElfStringTable.c

ELF_OBJS = ElfHackerMain.o \
			ElfHacker.o \
			ElfHeader.o \
			ElfProgHeader.o \
			ElfSectionHeader.o \
			ElfSymbolTable.o \
			ElfDynSymbolTable.o \
			ElfDynamicSection.o \
			ElfRelocationSection.o \
			ElfHashTable.o \
			ElfStringTable.o

elfhacker: ${ELF_OBJS}
		${CC} ${CFLAGS} ${ELF_OBJS} -o elfhacker

${ELF_OBJS} : ${ELF_HEADERS}

clean:
	rm -f *.o
	rm -f elfhacker
