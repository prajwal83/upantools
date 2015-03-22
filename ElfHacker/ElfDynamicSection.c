#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfDynamicSection.h"
#include "ElfConstants.h"
#include "ElfGlobals.h"

Elf32_DTType ElfDTTypes[NO_OF_DT_TYPES] ;

/************* Static Functions Start ***************/

static void AllocateDynamicSection()
{
	unsigned i ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_DYNAMIC)
		{
			ElfHacker_uiNoOfDynSectionEntries = ElfHacker_pELFSectionHeader[i].sh_size / ElfHacker_pELFSectionHeader[i].sh_entsize ;
			ElfHacker_pELFDynamicSection = (Elf32_Dyn*)malloc(ElfHacker_uiNoOfDynSectionEntries *  sizeof(Elf32_Dyn)) ;
			break ;
		}
	}
}

static void Initialize()
{
	strcpy(ElfDTTypes[ DT_NULL ].tag_name, "DT_NULL") ;
	ElfDTTypes[ DT_NULL ].addr_type = ADDR_TYPE_IGN ;

	strcpy(ElfDTTypes[ DT_NEEDED ].tag_name, "DT_NEEDED") ;
	ElfDTTypes[ DT_NEEDED ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_PLTRELSZ ].tag_name, "DT_PLTRELSZ") ;
	ElfDTTypes[ DT_PLTRELSZ ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_PLTGOT ].tag_name, "DT_PLTGOT") ;
	ElfDTTypes[ DT_PLTGOT ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_HASH ].tag_name, "DT_HASH") ;
	ElfDTTypes[ DT_HASH ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_STRTAB ].tag_name, "DT_STRTAB") ;
	ElfDTTypes[ DT_STRTAB ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_SYMTAB ].tag_name, "DT_SYMTAB") ;
	ElfDTTypes[ DT_SYMTAB ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_RELA ].tag_name, "DT_RELA") ;
	ElfDTTypes[ DT_RELA ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_RELASZ ].tag_name, "DT_RELASZ") ;
	ElfDTTypes[ DT_RELASZ ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_RELAENT ].tag_name, "DT_RELAENT") ;
	ElfDTTypes[ DT_RELAENT ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_STRSZ ].tag_name, "DT_STRSZ") ;
	ElfDTTypes[ DT_STRSZ ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_SYMENT ].tag_name, "DT_SYMENT") ;
	ElfDTTypes[ DT_SYMENT ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_INIT ].tag_name, "DT_INIT") ;
	ElfDTTypes[ DT_INIT ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_FINI ].tag_name, "DT_FINI") ;
	ElfDTTypes[ DT_FINI ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_SONAME ].tag_name, "DT_SONAME") ;
	ElfDTTypes[ DT_SONAME ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_RPATH ].tag_name, "DT_RPATH") ;
	ElfDTTypes[ DT_RPATH ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_SYMBOLIC ].tag_name, "DT_SYMBOLIC") ;
	ElfDTTypes[ DT_SYMBOLIC ].addr_type = ADDR_TYPE_IGN ;

	strcpy(ElfDTTypes[ DT_REL ].tag_name, "DT_REL") ;
	ElfDTTypes[ DT_REL ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_RELSZ ].tag_name, "DT_RELSZ") ;
	ElfDTTypes[ DT_RELSZ ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_RELENT ].tag_name, "DT_RELENT") ;
	ElfDTTypes[ DT_RELENT ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_PLTREL ].tag_name, "DT_PLTREL") ;
	ElfDTTypes[ DT_PLTREL ].addr_type = ADDR_TYPE_VAL ;

	strcpy(ElfDTTypes[ DT_DEBUG ].tag_name, "DT_DEBUG") ;
	ElfDTTypes[ DT_DEBUG ].addr_type = ADDR_TYPE_PTR ;

	strcpy(ElfDTTypes[ DT_TEXTREL ].tag_name, "DT_TEXTREL") ;
	ElfDTTypes[ DT_TEXTREL ].addr_type = ADDR_TYPE_IGN ;

	strcpy(ElfDTTypes[ DT_JMPREL ].tag_name, "DT_JMPREL") ;
	ElfDTTypes[ DT_JMPREL ].addr_type = ADDR_TYPE_PTR ;
}

static void DisplayDynamicSectionEntry(Elf32_Dyn* dynEntry)
{
	if(dynEntry->d_tag >= DT_LOPROC && dynEntry->d_tag <= DT_HIPROC)
	{
		printf("\t\tDT_TYPE = Unspecified") ;
	}
	else
	{
		printf("   %x   ", dynEntry->d_tag) ;
		fflush(stdout) ;
		printf("\t\tDT_TYPE = %s", ElfDTTypes[dynEntry->d_tag].tag_name) ;
		
		switch(ElfDTTypes[dynEntry->d_tag].addr_type)
		{
			case ADDR_TYPE_IGN:
				printf("\t\tDT_ADDR = IGNORED") ;
				break ;

			case ADDR_TYPE_VAL:
				printf("\t\tDT_ADDR(val) = %u", dynEntry->d_un.d_val) ;
				break ;

			case ADDR_TYPE_PTR:
				printf("\t\tDT_ADDR(ptr) = %u", dynEntry->d_un.d_ptr) ;
				break ;
		}
	}
}

/************* Static Functions End ***************/

void ElfDynamicSection_DeAllocate()
{
	free(ElfHacker_pELFDynamicSection) ;
}

int ElfDynamicSection_Read()
{
	unsigned i ;

	Initialize() ;
	AllocateDynamicSection() ;

	for(i = 0; i < ElfHacker_elfHeader.e_shnum; i++)
	{
		if(ElfHacker_pELFSectionHeader[i].sh_type == SHT_DYNAMIC)
		{
			if(lseek(ElfHacker_File_fd, ElfHacker_pELFSectionHeader[i].sh_offset, SEEK_SET) < 0)
			{
				perror("LSEEK DYM TABLE") ;
				return -1 ;
			}

			if(read(ElfHacker_File_fd, (char*)(ElfHacker_pELFDynamicSection), ElfHacker_uiNoOfDynSectionEntries * sizeof(Elf32_Dyn)) < 0)
			{
				perror("READ DYM TABLE") ;
				return -1 ;
			}
			break ;
		}
	}

	return 0 ;
}

void ElfDynamicSection_Display()
{
	printf("\n************ ELF DYNAMIC SECTION START **********************\n") ;

	unsigned i ;

	printf("\n No of Ent = %u", ElfHacker_uiNoOfDynSectionEntries) ;
	for(i = 0; i < ElfHacker_uiNoOfDynSectionEntries; i++)
	{
		if(ElfHacker_pELFDynamicSection[i].d_tag == DT_NULL)
			break ;
		printf("\n Dynamic Section Entry %d:", i) ;
		DisplayDynamicSectionEntry(&ElfHacker_pELFDynamicSection[i]) ;
	}

	printf("\n************ ELF DYNAMIC SECTION END **********************\n") ;
}

