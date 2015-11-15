#ifndef _ELF_SYM_TAB_HEADER_H_
#define _ELF_SYM_TAB_HEADER_H_

#include "ElfConstants.h"

/************************** Elf Symbol Table Start *****************************************************/
typedef	struct	{
	Elf32_Word		st_name;
	Elf32_Addr		st_value;
	Elf32_Word		st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half		st_shndx;
} Elf32_Sym ;

/******* st_info Extractor ******/

#define ELF32_ST_BIND(st_info) ( (st_info) >> 4 )
#define ELF32_ST_TYPE(st_info) ( (st_info) & 0xf )
#define ELF32_ST_INFO(bind, type) ( ((bind) << 4) + ((type) & 0xf) )

/**** Symbol Binding Types *******/

#define STB_LOCAL	0
#define STB_GLOBAL	1
#define STB_WEAK	2
#define STB_LOPROC	13
#define STB_HIPROC	15

/***** Symbol Types *******/

#define STT_NOTYPE	0
#define STT_OBJECT	1
#define STT_FUNC	2
#define STT_SECTION	3
#define STT_FILE	4
#define STT_LOPROC	13
#define STT_HIPROC	15

typedef struct {

	unsigned uiTableSize ;
	Elf32_Sym* SymTabEntries ;

} ElfParser_SymbolTableList ;

int ElfSymbolTable_Read() ;
void ElfSymbolTable_Display() ;
void ElfSymbolTable_DeAllocate() ;

int ElfDynSymbolTable_Read() ;
void ElfDynSymbolTable_Display() ;
void ElfDynSymbolTable_DeAllocate() ;

/************************** Elf Symbol Table End *****************************************************/

#endif
