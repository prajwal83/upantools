#ifndef _REL_SECTION_H_
#define _REL_SECTION_H_

#include "ElfConstants.h"

typedef struct {
	Elf32_Addr r_offset ;
	Elf32_Word r_info ;
} Elf32_Rel ;

typedef struct {
	Elf32_Addr r_offset ;
	Elf32_Word r_info ;
	Elf32_Sword r_addend ;
} Elf32_Rela ;

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char)(t))

#define R_386_NONE		0
#define R_386_32		1
#define R_386_PC32		2
#define R_386_GOT32 	3
#define R_386_PLT32		4
#define R_386_COPY		5
#define R_386_GLOB_DAT	6
#define R_386_JMP_SLOT	7
#define R_386_RELATIVE	8
#define R_386_GOTOFF	9
#define R_386_GOTPC		10

static const char RelocationType[11][40] = {
	"R_386_NONE",
	"R_386_32",
	"R_386_PC32",
	"R_386_GOT32 ",
	"R_386_PLT32",
	"R_386_COPY",
	"R_386_GLOB_DAT",
	"R_386_JMP_SLOT",
	"R_386_RELATIVE",
	"R_386_GOTOFF",
	"R_386_GOTPC",
} ;

void ElfRelocationSection_DeAllocateWithOutAddends() ;
void ElfRelocationSection_DeAllocateWithAddends() ;
int ElfRelocationSection_Read() ;
int ElfRelocationSection_ReadAd() ;
void ElfRelocationSection_DisplayWithOutAddends() ;
void ElfRelocationSection_DisplayWithAddends() ;
int ElfDynRelocationSection_Read() ;
void ElfRelocationSection_DisplayDyn() ;

#endif
