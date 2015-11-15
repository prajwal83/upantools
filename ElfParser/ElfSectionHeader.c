#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "ElfConstants.h"
#include "ElfHeader.h"
#include "ElfProgHeader.h"
#include "ElfSectionHeader.h"
#include "ElfParser.h"
#include "ElfSymbolTable.h"
#include "ElfGlobals.h"

/************* Static Functions Start ***************/

static void AllocateSectionHeader()
{
	ElfParser_pELFSectionHeader = (Elf32_Shdr*)malloc(sizeof(Elf32_Shdr) * ElfParser_elfHeader.e_shnum) ;
}

void ElfSectionHeader_DeAllocate()
{
	free(ElfParser_pELFSectionHeader) ;
}

static const char* GetSectionFlags(int sflag)
{
	char blank[10] = "-----" ;
	strcpy(sec_flags, "") ;

	if(sflag > (SHF_EXECINSTR | SHF_ALLOC | SHF_WRITE) || sflag < 1)
	{
		strcat(sec_flags, "Processor Specific") ;
		return sec_flags ;
	}

	if((sflag & SHF_WRITE) == SHF_WRITE)
		strcat(sec_flags, "Write-") ;
	else
		strcat(sec_flags, blank) ;
		
	if((sflag & SHF_ALLOC) == SHF_ALLOC)
		strcat(sec_flags, "Alloc-") ;
	else
		strcat(sec_flags, blank) ;

	if((sflag & SHF_EXECINSTR) == SHF_EXECINSTR)
		strcat(sec_flags, "Execute") ;
	else
		strcat(sec_flags, blank) ;
		
	return sec_flags ;
}


/************* Static Functions End ***************/

int ElfSectionHeader_Read()
{
	/* e_shentsize if not used as this parser is only for 32 bit elf files.
	   and ElfSectionHeader size is 40 bytes */
	   
	AllocateSectionHeader() ;
	
	if(lseek(ElfParser_File_fd, ElfParser_elfHeader.e_shoff, SEEK_SET) < 0)
	{
		perror("LSEEK") ;
		return -1 ;
	}
	
	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		if(read(ElfParser_File_fd, (char*)&ElfParser_pELFSectionHeader[i], sizeof(Elf32_Shdr)) < 0)
		{
			perror("READ ELF SEC_HEADER") ;
			return -1 ;
		}
	}
	
	return 0 ;
}

void ElfSectionHeader_Display()
{
	printf("\n************ ELF SECTION HEADERS START **********************\n") ;

	if(ElfParser_pELFSectionHeader == NULL)
	{
		printf("\n ELF Section Header is not populated\n") ;
		return ;
	}

	unsigned i ;
	for(i = 0; i < ElfParser_elfHeader.e_shnum; i++)
	{
		printf("\n\nELF Section Header %d", i) ;
		printf("\n\tsh_name = %s", GET_SECTION_NAME(ElfParser_pELFSectionHeader[i].sh_name)) ;
		printf("\n\tsh_type = %s", GET_SEC_HEADER_TYPE(ElfParser_pELFSectionHeader[i].sh_type)) ;
		printf("\n\tsh_flags = %s", GetSectionFlags(ElfParser_pELFSectionHeader[i].sh_flags)) ;
		printf("\n\tsh_addr = 0x%X (%d)", ElfParser_pELFSectionHeader[i].sh_addr, ElfParser_pELFSectionHeader[i].sh_addr) ;
		printf("\n\tsh_offset = %d", ElfParser_pELFSectionHeader[i].sh_offset) ;
		printf("\n\tsh_size = %d", ElfParser_pELFSectionHeader[i].sh_size) ;
		printf("\n\tsh_link = %d", ElfParser_pELFSectionHeader[i].sh_link) ;
		printf("\n\tsh_info = %d", ElfParser_pELFSectionHeader[i].sh_info) ;
		printf("\n\tsh_addralign = %d", ElfParser_pELFSectionHeader[i].sh_addralign) ;
		printf("\n\tsh_entsize = %d", ElfParser_pELFSectionHeader[i].sh_entsize) ;
	}
	
	printf("\n************ ELF SECTION HEADERS END ************************\n") ;
}

