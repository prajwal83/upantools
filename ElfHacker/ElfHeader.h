
#ifndef _ELF_HEADER_H_
#define _ELF_HEADER_H_

#define EI_NIDENT 16

/******* Elf Header **********/
typedef struct 
{
	unsigned char  e_ident[EI_NIDENT];
	uint16_t       e_type;
	uint16_t       e_machine;
	uint32_t       e_version;
	Elf32_Addr     e_entry;
	Elf32_Off      e_phoff;
	Elf32_Off      e_shoff;
	uint32_t       e_flags;
	uint16_t       e_ehsize;
	uint16_t       e_phentsize;
	uint16_t       e_phnum;
	uint16_t       e_shentsize;
	uint16_t       e_shnum;
	uint16_t       e_shstrndx;
} Elf_Header ;

/************ Elf Header Interpretation ******************/
/****** Index for ei_ident field of Elf Headers ******/

#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_PAD 7

static const char Elf_Classes[3][20] = {
"Invalid class",
"32-bit objects",
"64-bit objects"
} ;

#define ELFCLASSNONE 0 
#define ELFCLASS32 1 
#define ELFCLASS64 2

static const char Elf_Versions[2][20] = {
"Invalid Version",
"Current Version"
} ;

#define EV_NONE 0
#define EV_CURRENT 1

static const char Elf_DataEncoding[3][30] = {
"Invalid Data Encoding",
"2's Comp LSB",
"2's Comp MSB"
} ;

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

static const char Elf_eType[7][50] = {
"No file type",
"Relocatable file",
"Executable file",
"Shared object file",
"Core file",
"0xff00 Processor-specific",
"0xffff Processor-specific"
} ;

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOPROC 5
#define ET_HIPROC 6

static const char Elf_eMachine[13][30] = {
"No machine",
"AT&T WE 32100",
"SPARC",
"Intel 80386",
"Motorola 68000",
"Motorola 88000",
UNKNOWN,
"Intel 80860",
"MIPS RS3000",
UNKNOWN,
UNKNOWN,
UNKNOWN,
UNKNOWN
} ;

#define EM_NONE 0
#define EM_M32 1
#define EM_SPARC 2
#define EM_386 3
#define EM_68K 4
#define EM_88K 5
#define EM_860 7
#define EM_MIPS 8

int ElfHeader_Read() ;
void ElfHeader_Display() ;

#endif
