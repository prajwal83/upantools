#ifndef _ELF_SEC_HEADER_H_
#define _ELF_SEC_HEADER_H_

/************************** Elf Section Header Start *****************************************************/
typedef struct {
	uint32_t		sh_name;
	uint32_t		sh_type;
	uint32_t		sh_flags;
	Elf32_Addr		sh_addr;
	Elf32_Off		sh_offset;
	uint32_t		sh_size;
	uint32_t		sh_link;
	uint32_t		sh_info;
	uint32_t		sh_addralign;
	uint32_t		sh_entsize;
} Elf32_Shdr;

/* Reserved Section Header Table Indexes */

#define SHN_UNDEF		0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC		0xff00
#define SHN_HIPROC		0xff1f
#define SHN_ABS			0xfff1
#define SHN_COMMON		0xfff2
#define SHN_HIRESERVE	0xffff


/* Section Types */
#define SHT_NULL		0
#define SHT_PROGBITS	1
#define SHT_SYMTAB		2
#define SHT_STRTAB		3
#define SHT_RELA		4
#define SHT_HASH		5
#define SHT_DYNAMIC		6
#define SHT_NOTE		7
#define SHT_NOBITS		8
#define SHT_REL			9
#define SHT_SHLIB		10
#define SHT_DYNSYM		11
#define SHT_LOPROC		0x70000000
#define SHT_HIPROC		0x7fffffff
#define SHT_LOUSER		0x80000000
#define SHT_HIUSER		0xffffffff

static const char Elf_SectionHeaderType[12][40] = {
"Null Section",
"PROGBITS",
"Symbol Table Section",
"String Table Section",
"Relocation Info With addends",
"Hash Table Section",
"Dynamic Link Info",
"Note Section",
"NO BITS",
"Relocation Info WithOut addends",
"SHLIB Unspecified",
"Dynamic Symbol Table Section",
} ;

#define GET_SEC_HEADER_TYPE(stype) (0 <= stype && stype <= 11) ? Elf_SectionHeaderType[stype] : \
							(stype >= PT_LOPROC && stype <= PT_HIPROC) ? "Processor-Specific" : \
							"Invalid Section Header Type" 

/* Section Flags */
#define SHF_WRITE		0x1
#define SHF_ALLOC		0x2
#define SHF_EXECINSTR	0x4
#define SHF_MASKPROC	0xf0000000

char sec_flags[40] ;

#define GET_SECTION_NAME(str_index) ElfParser_pSecHeaderStrTable + str_index

int ElfSectionHeader_Read() ;
void ElfSectionHeader_Display() ;
void ElfSectionHeader_DeAllocate() ;

/************************** Elf Section Header End  *****************************************************/

#endif
