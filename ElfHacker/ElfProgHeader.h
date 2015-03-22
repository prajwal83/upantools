#ifndef _ELF_PROG_HEADER_H_
#define _ELF_PROG_HEADER_H_

/************************** Elf Program Header Start *****************************************************/

typedef struct {
	uint32_t	p_type ;
	Elf32_Off	p_offset ;
	Elf32_Addr	p_vaddr ;
	Elf32_Addr	p_paddr ;
	uint32_t	p_filesz ;
	uint32_t	p_memsz ;
	uint32_t	p_flags ;
	uint32_t	p_align ;
} Elf32_Phdr ;


#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff 

static const char Elf_ProgramHeaderType[7][30] = {
"Null Program Header",
"Loadable Segment",
"Dynamic Linking Info",
"Interpreter Info",
"Auxiliary Info",
"SHLIB Unspecified",
"Program Header Info"
} ;

#define PF_R 0x4 
#define PF_W 0x2
#define PF_X 0x1

char seg_access[40] ;

#define GET_PROG_HEADER_TYPE(ptype) (0 <= ptype && ptype <= 6) ? Elf_ProgramHeaderType[ptype] : \
							(ptype <= PT_LOPROC || ptype >= PT_HIPROC) ? "Processor-Specific" : \
							"Invalid Program Header Type" 

int ElfProgHeader_Read() ;
void ElfProgHeader_Display() ;
void ElfProgHeader_DeAllocate() ;

/************************** Elf Program Header End  *****************************************************/

#endif
