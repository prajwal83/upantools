#ifndef _DYNAMIC_SECTION_H_
#define _DYNAMIC_SECTION_H_

#include "ElfConstants.h"

typedef struct
{
	Elf32_Sword	d_tag ;

	union 
	{
		Elf32_Word	d_val ;
		Elf32_Addr	d_ptr ;
	} d_un ;

} Elf32_Dyn ;

typedef struct
{
	char tag_name[30] ;
	unsigned addr_type ;
} Elf32_DTType ;

#define ADDR_TYPE_IGN	0
#define ADDR_TYPE_VAL	1
#define ADDR_TYPE_PTR	2

#define DT_NULL			0
#define DT_NEEDED		1
#define DT_PLTRELSZ		2
#define DT_PLTGOT		3
#define DT_HASH			4
#define DT_STRTAB		5
#define DT_SYMTAB		6
#define DT_RELA			7
#define DT_RELASZ		8
#define DT_RELAENT		9
#define DT_STRSZ		10
#define DT_SYMENT		11
#define DT_INIT			12
#define DT_FINI			13
#define DT_SONAME		14
#define DT_RPATH		15
#define DT_SYMBOLIC		16
#define DT_REL			17
#define DT_RELSZ		18
#define DT_RELENT		19
#define DT_PLTREL		20
#define DT_DEBUG		21
#define DT_TEXTREL		22
#define DT_JMPREL		23
#define DT_LOPROC		0x60000000
#define DT_HIPROC		0x7fffffff 

#define NO_OF_DT_TYPES 24

void ElfDynamicSection_DeAllocate() ;
int ElfDynamicSection_Read() ;
void ElfDynamicSection_Display() ;

#endif
